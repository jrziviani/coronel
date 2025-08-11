#include "user_allocator.hpp"
#include "memory_manager.hpp"
#include "arch/amd64/memory/paging.hpp"
#include "libs/string.hpp"
#include "libs/logger.hpp"
#include "config.hpp"

namespace memory {

    user_allocator::user_allocator(paddr_t page_dir) :
        heap_start_(get_user_heap_start()),
        heap_current_(get_user_heap_start()),
        heap_limit_(get_user_heap_max()),
        first_block_(nullptr),
        page_directory_(page_dir),
        total_allocated_(0),
        num_blocks_(0)
    {
        // Initial heap setup - request initial heap pages from kernel
        if (sys_request_memory(heap_start_, USER_HEAP_INITIAL_SIZE)) {
            heap_current_ = reinterpret_cast<vaddr_t>(ptr_from(heap_start_) + USER_HEAP_INITIAL_SIZE);
            
            // Create initial free block
            first_block_ = reinterpret_cast<user_block*>(heap_start_);
            new (first_block_) user_block(USER_HEAP_INITIAL_SIZE - sizeof(user_block), true);
        }
    }

    user_allocator::~user_allocator()
    {
        cleanup_on_exit();
    }

    bool user_allocator::sys_request_memory(vaddr_t addr, size_t size)
    {
        // This would be implemented as a system call in a real kernel
        // For now, we'll use the kernel's paging system directly
        
        paging page_mgr;
        size_t pages_needed = ALIGN_UP(size) / FRAME_SIZE;
        
        for (size_t i = 0; i < pages_needed; i++) {
            // Allocate physical frame
            paddr_t phys_addr = g_physical_manager->alloc();
            if (phys_addr == nullptr) {
                // Cleanup allocated pages on failure
                for (size_t j = 0; j < i; j++) {
                    vaddr_t cleanup_addr = reinterpret_cast<vaddr_t>(ptr_from(addr) + j * FRAME_SIZE);
                    page_mgr.unmap(page_directory_, cleanup_addr);
                }
                return false;
            }
            
            // Map with user permissions (Present + Writable + User)
            vaddr_t virt_addr = reinterpret_cast<vaddr_t>(ptr_from(addr) + i * FRAME_SIZE);
            int result = page_mgr.map(page_directory_, virt_addr, phys_addr, 0x07);
            
            if (result != 0) {
                g_physical_manager->free(phys_addr);
                // Cleanup on failure
                for (size_t j = 0; j < i; j++) {
                    vaddr_t cleanup_addr = reinterpret_cast<vaddr_t>(ptr_from(addr) + j * FRAME_SIZE);
                    page_mgr.unmap(page_directory_, cleanup_addr);
                }
                return false;
            }
        }
        
        return true;
    }

    void user_allocator::sys_release_memory(vaddr_t addr, size_t size)
    {
        paging page_mgr;
        size_t pages = ALIGN_UP(size) / FRAME_SIZE;
        
        for (size_t i = 0; i < pages; i++) {
            vaddr_t virt_addr = reinterpret_cast<vaddr_t>(ptr_from(addr) + i * FRAME_SIZE);
            page_mgr.unmap(page_directory_, virt_addr);
        }
    }

    user_block* user_allocator::find_free_block(uint32_t size)
    {
        user_block* current = first_block_;
        
        while (current != nullptr) {
            if (current->is_free() && current->size >= size) {
                return current;
            }
            current = current->next;
        }
        
        return nullptr;
    }

    bool user_allocator::expand_heap(uint32_t min_size)
    {
        // Calculate expansion size (align to pages)
        size_t expand_size = ALIGN_UP(min_size);
        
        // Check heap limit
        if (ptr_from(heap_current_) + expand_size > ptr_from(get_user_heap_max())) {
            return false; // Hit heap limit
        }
        
        // Request memory from kernel
        if (!sys_request_memory(heap_current_, expand_size)) {
            return false;
        }
        
        // Create new free block
        user_block* new_block = reinterpret_cast<user_block*>(heap_current_);
        new (new_block) user_block(expand_size - sizeof(user_block), true);
        
        // Link into block list
        if (first_block_ != nullptr) {
            user_block* last = first_block_;
            while (last->next != nullptr) {
                last = last->next;
            }
            last->next = new_block;
            new_block->prev = last;
        } else {
            first_block_ = new_block;
        }
        
        heap_current_ = reinterpret_cast<vaddr_t>(ptr_from(heap_current_) + expand_size);
        
        // Try to coalesce with previous block
        coalesce_block(new_block);
        
        return true;
    }

    void user_allocator::split_block(user_block* block, uint32_t size)
    {
        if (block->size < size + sizeof(user_block) + 16) {
            return; // Not worth splitting
        }
        
        // Create new block for remaining space
        vaddr_t new_block_addr = reinterpret_cast<vaddr_t>(ptr_from(block->data()) + size);
        user_block* new_block = reinterpret_cast<user_block*>(new_block_addr);
        
        uint32_t remaining_size = block->size - size - sizeof(user_block);
        new (new_block) user_block(remaining_size, true);
        
        // Update links
        new_block->next = block->next;
        new_block->prev = block;
        if (block->next) {
            block->next->prev = new_block;
        }
        block->next = new_block;
        
        // Update original block
        block->size = size;
        num_blocks_++;
    }

    void user_allocator::coalesce_block(user_block* block)
    {
        if (!block->is_free()) {
            return;
        }
        
        // Coalesce with next block
        if (block->next && block->next->is_free()) {
            uintptr_t block_end = ptr_from(block->data()) + block->size;
            uintptr_t next_start = ptr_from(block->next);
            
            if (block_end == next_start) {
                user_block* next_block = block->next;
                block->size += sizeof(user_block) + next_block->size;
                block->next = next_block->next;
                if (next_block->next) {
                    next_block->next->prev = block;
                }
                num_blocks_--;
            }
        }
        
        // Coalesce with previous block
        if (block->prev && block->prev->is_free()) {
            uintptr_t prev_end = ptr_from(block->prev->data()) + block->prev->size;
            uintptr_t block_start = ptr_from(block);
            
            if (prev_end == block_start) {
                user_block* prev_block = block->prev;
                prev_block->size += sizeof(user_block) + block->size;
                prev_block->next = block->next;
                if (block->next) {
                    block->next->prev = prev_block;
                }
                num_blocks_--;
            }
        }
    }

    void* user_allocator::malloc(size_t size)
    {
        if (size == 0) {
            return nullptr;
        }
        
        // For 64-bit systems, we still limit individual allocations to 2GB
        // to prevent issues with 32-bit block headers and ensure reasonable limits
        constexpr size_t MAX_SINGLE_ALLOC = 0x80000000ULL; // 2GB
        if (size > MAX_SINGLE_ALLOC) {
            return nullptr; // Allocation too large
        }
        
        uint32_t aligned_size = ALIGN_UP(size);
        
        // Find free block
        user_block* block = find_free_block(aligned_size);
        
        // Expand heap if needed
        if (block == nullptr) {
            if (!expand_heap(aligned_size + sizeof(user_block))) {
                return nullptr;
            }
            block = find_free_block(aligned_size);
        }
        
        if (block == nullptr) {
            return nullptr;
        }
        
        // Split block if beneficial
        split_block(block, aligned_size);
        
        // Mark as used
        block->set_free(false);
        total_allocated_ += aligned_size;
        
        return block->data();
    }

    void user_allocator::free(void* ptr)
    {
        if (ptr == nullptr) {
            return;
        }
        
        user_block* block = user_block::from_data(ptr);
        
        // Validate block
        if (!block->is_valid() || block->is_free()) {
            return; // Invalid or double free
        }
        
        // Mark as free
        block->set_free(true);
        total_allocated_ -= block->size;
        
        // Coalesce adjacent blocks
        coalesce_block(block);
    }

    void* user_allocator::realloc(void* ptr, size_t new_size)
    {
        if (ptr == nullptr) {
            return malloc(new_size);
        }
        
        if (new_size == 0) {
            free(ptr);
            return nullptr;
        }
        
        user_block* block = user_block::from_data(ptr);
        if (!block->is_valid() || block->is_free()) {
            return nullptr;
        }
        
        uint32_t aligned_new_size = ALIGN_UP(new_size);
        uint32_t old_size = block->size;
        
        // If new size fits, we're done
        if (aligned_new_size <= old_size) {
            if (old_size > aligned_new_size + sizeof(user_block) + 16) {
                split_block(block, aligned_new_size);
            }
            return ptr;
        }
        
        // Need new block
        void* new_ptr = malloc(new_size);
        if (new_ptr == nullptr) {
            return nullptr;
        }
        
        // Copy data
        lib::memcpy(new_ptr, ptr, old_size);
        free(ptr);
        
        return new_ptr;
    }

    void* user_allocator::calloc(size_t num, size_t size)
    {
        // Check for overflow in multiplication
        constexpr size_t MAX_TOTAL_ALLOC = 0x80000000ULL; // 2GB max
        if (num > 0 && (size > MAX_TOTAL_ALLOC / num)) {
            return nullptr; // Overflow or too large
        }
        
        size_t total_size = num * size;
        void* ptr = malloc(total_size);
        
        if (ptr != nullptr) {
            lib::memset(ptr, 0, total_size);
        }
        
        return ptr;
    }

    bool user_allocator::set_heap_limit(size_t max_size)
    {
        vaddr_t new_limit = reinterpret_cast<vaddr_t>(ptr_from(heap_start_) + max_size);
        
        if (new_limit > get_user_heap_max() || new_limit < heap_current_) {
            return false;
        }
        
        heap_limit_ = new_limit;
        return true;
    }

    size_t user_allocator::get_heap_size() const
    {
        return ptr_from(heap_current_) - ptr_from(heap_start_);
    }

    bool user_allocator::validate_heap() const
    {
        user_block* current = first_block_;
        uint32_t counted_allocated = 0;
        
        while (current != nullptr) {
            if (!current->is_valid()) {
                return false;
            }
            
            if (!current->is_free()) {
                counted_allocated += current->size;
            }
            
            current = current->next;
        }
        
        return counted_allocated == total_allocated_;
    }

    void user_allocator::cleanup_on_exit()
    {
        // Release all heap memory back to kernel
        if (heap_current_ > heap_start_) {
            size_t total_size = ptr_from(heap_current_) - ptr_from(heap_start_);
            sys_release_memory(heap_start_, total_size);
        }
    }

    // Process memory management implementation
    process_memory::process_memory(paddr_t page_dir) :
        page_directory_(page_dir),
        virtual_manager_(),
        heap_(nullptr),
        stack_top_(nullptr),
        stack_size_(0),
        code_start_(nullptr),
        code_size_(0),
        data_start_(nullptr),
        data_size_(0)
    {
        heap_ = new (memory::kmalloc(sizeof(user_allocator))) user_allocator(page_dir);
    }

    process_memory::~process_memory()
    {
        cleanup_all();
    }

    bool process_memory::setup_memory_layout(vaddr_t code_addr, size_t code_sz,
                                            vaddr_t data_addr, size_t data_sz)
    {
        code_start_ = code_addr;
        code_size_ = code_sz;
        data_start_ = data_addr;
        data_size_ = data_sz;
        
        // TODO: map these sections with appropriate permissions
        // Code: Read + Execute
        // Data: Read + Write
        
        return true;
    }

    bool process_memory::setup_stack(size_t stack_size)
    {
        stack_size_ = ALIGN_UP(stack_size);
        
        // User stack grows downward from high address (standard layout)
        stack_top_ = get_user_stack_top();
        vaddr_t stack_bottom = reinterpret_cast<vaddr_t>(ptr_from(stack_top_) - stack_size_);
        
        // Map stack pages
        paging page_mgr;
        size_t pages_needed = stack_size_ / FRAME_SIZE;
        
        for (size_t i = 0; i < pages_needed; i++) {
            paddr_t phys_addr = g_physical_manager->alloc();
            if (phys_addr == nullptr) {
                return false;
            }
            
            vaddr_t stack_page = reinterpret_cast<vaddr_t>(ptr_from(stack_bottom) + i * FRAME_SIZE);
            int result = page_mgr.map(page_directory_, stack_page, phys_addr, 0x07); // User R/W
            
            if (result != 0) {
                g_physical_manager->free(phys_addr);
                return false;
            }
        }
        
        return true;
    }

    void process_memory::cleanup_all()
    {
        if (heap_ != nullptr) {
            heap_->cleanup_on_exit();
            memory::kfree(heap_);
            heap_ = nullptr;
        }
        
        // TODO: unmap all process pages here
    }

    bool process_memory::validate_user_pointer(void* ptr, size_t size) const
    {
        uintptr_t addr = ptr_from(ptr);
        uintptr_t end_addr = addr + size;
        
        // Check for overflow
        if (end_addr < addr) {
            return false;
        }
        
        // Check if address is in NULL guard area (security)
        if (addr < USER_NULL_GUARD_SIZE) {
            return false; // NULL pointer protection
        }
        
        // Check if address is in valid user space range (64-bit)
        // Kernel space starts at 0xffffffff80000000, so user space is below that
        if (addr >= 0x0000800000000000ULL || end_addr >= 0x0000800000000000ULL) {
            return false; // Accessing kernel space
        }
        
        // TODO: check page table permissions here
        return true;
    }

    // System call implementations
    namespace syscalls {
        extern "C" void* sys_malloc(size_t size)
        {
            // TODO: get the current process context
            lib::log(lib::log_level::INFO, "sys_malloc called");
            return nullptr; // call current_process->memory->heap->malloc(size)
        }

        extern "C" void sys_free(void* ptr)
        {
            // current_process->memory->heap->free(ptr)
            lib::log(lib::log_level::INFO, "sys_free called");
        }

        extern "C" void* sys_realloc(void* ptr, size_t new_size)
        {
            // current_process->memory->heap->realloc(ptr, new_size)
            lib::log(lib::log_level::INFO, "sys_realloc called");
            return nullptr;
        }

        extern "C" void* sys_calloc(size_t num, size_t size)
        {
            // current_process->memory->heap->calloc(num, size)
            lib::log(lib::log_level::INFO, "sys_calloc called");
            return nullptr;
        }

        extern "C" int sys_brk(void* addr)
        {
            lib::log(lib::log_level::INFO, "sys_brk called");
            return -1;
        }

        extern "C" void* sys_mmap(void* addr, size_t length, int prot, int flags)
        {
            // Memory mapping system call
            lib::log(lib::log_level::INFO, "sys_mmap called");
            return nullptr;
        }

        extern "C" int sys_munmap(void* addr, size_t length)
        {
            // Unmap memory
            lib::log(lib::log_level::INFO, "sys_munmap called");
            return -1;
        }
    }
}