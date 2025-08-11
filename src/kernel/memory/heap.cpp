#include "heap.hpp"
#include "allocators.hpp"
#include "config.hpp"
#include "libs/logger.hpp"
#include "libs/string.hpp"
#include "arch/amd64/memory/paging.hpp"

namespace memory {

    // Helper macros
    #define HEAP_ALIGN_UP(size) ALIGN_UP(size)

    // Global heap instance
    heap* g_kernel_heap = nullptr;

    heap::heap(physical* phys, virt* virt_mgr, vaddr_t start, size_t initial_size) :
        phys_manager_(phys),
        virt_manager_(virt_mgr),
        heap_start_(start),
        heap_size_(initial_size),
        first_block_(nullptr),
        total_allocated_(0),
        total_free_(0),
        num_allocations_(0),
        num_frees_(0)
    {
        // Align heap size to page boundary
        heap_size_ = HEAP_ALIGN_UP(heap_size_);
        heap_end_ = reinterpret_cast<vaddr_t>(ptr_from(heap_start_) + heap_size_);
        
        // Map initial heap pages
        if (!map_pages(heap_start_, heap_size_)) {
            lib::log(lib::log_level::CRITICAL, "Failed to map initial heap pages");
            return;
        }
        
        // Create initial free block covering the entire heap
        first_block_ = reinterpret_cast<heap_block*>(heap_start_);
        new (first_block_) heap_block(heap_size_ - sizeof(heap_block), true);
        
        total_free_ = heap_size_ - sizeof(heap_block);
        
        lib::log(lib::log_level::INFO, "Kernel heap initialized");
    }

    heap::~heap()
    {
        // Unmap all heap pages
        unmap_pages(heap_start_, heap_size_);
    }

    bool heap::map_pages(vaddr_t start, size_t size)
    {
        size_t pages_needed = HEAP_ALIGN_UP(size) / FRAME_SIZE;
        paging page_manager; // We'll use the paging class to map pages
        
        for (size_t i = 0; i < pages_needed; i++) {
            // Allocate physical frame
            paddr_t phys_addr = phys_manager_->alloc();
            if (phys_addr == nullptr) {
                lib::log(lib::log_level::CRITICAL, "Failed to allocate physical frame for heap");
                
                // Cleanup already mapped pages
                for (size_t j = 0; j < i; j++) {
                    vaddr_t cleanup_vaddr = reinterpret_cast<vaddr_t>(ptr_from(start) + j * FRAME_SIZE);
                    page_manager.unmap(cleanup_vaddr);
                }
                return false;
            }
            
            // Map virtual address to physical frame
            vaddr_t virt_addr = reinterpret_cast<vaddr_t>(ptr_from(start) + i * FRAME_SIZE);
            int result = page_manager.map(virt_addr, phys_addr, 0x03); // Present + Writable
            
            if (result != 0) {
                lib::log(lib::log_level::CRITICAL, "Failed to map virtual address for heap");
                
                // Free the physical frame we just allocated
                phys_manager_->free(phys_addr);
                
                // Cleanup already mapped pages
                for (size_t j = 0; j < i; j++) {
                    vaddr_t cleanup_vaddr = reinterpret_cast<vaddr_t>(ptr_from(start) + j * FRAME_SIZE);
                    page_manager.unmap(cleanup_vaddr);
                }
                return false;
            }
            
            // Clear the page
            lib::memset(reinterpret_cast<void*>(virt_addr), 0, FRAME_SIZE);
        }
        
        return true;
    }

    void heap::unmap_pages(vaddr_t start, size_t size)
    {
        size_t pages = HEAP_ALIGN_UP(size) / FRAME_SIZE;
        paging page_manager;
        
        for (size_t i = 0; i < pages; i++) {
            vaddr_t virt_addr = reinterpret_cast<vaddr_t>(ptr_from(start) + i * FRAME_SIZE);
            page_manager.unmap(virt_addr);
        }
    }

    bool heap::expand_heap(size_t min_size)
    {
        // Calculate how much to expand (at least min_size, but align to pages)
        size_t expand_size = HEAP_ALIGN_UP(min_size);
        
        // Allocate virtual memory for expansion
        vaddr_t new_region = virt_manager_->alloc(expand_size);
        if (new_region == nullptr) {
            lib::log(lib::log_level::CRITICAL, "Failed to allocate virtual memory for heap expansion");
            return false;
        }
        
        // Map physical pages to the new virtual region
        if (!map_pages(new_region, expand_size)) {
            virt_manager_->free(new_region, expand_size);
            return false;
        }
        
        // Create a new free block in the expanded region
        heap_block* new_block = reinterpret_cast<heap_block*>(new_region);
        new (new_block) heap_block(expand_size - sizeof(heap_block), true);
        
        // Link it into our block list (add to end)
        heap_block* current = first_block_;
        while (current->next != nullptr) {
            current = current->next;
        }
        current->next = new_block;
        new_block->prev = current;
        
        heap_size_ += expand_size;
        total_free_ += expand_size - sizeof(heap_block);
        
        // Try to coalesce with previous block if adjacent
        coalesce_block(new_block);
        
        lib::log(lib::log_level::INFO, "Heap expanded");
        return true;
    }

    heap_block* heap::find_free_block(size_t size)
    {
        heap_block* current = first_block_;
        
        while (current != nullptr) {
            if (current->is_free && current->size >= size) {
                return current;
            }
            current = current->next;
        }
        
        return nullptr;
    }

    void heap::split_block(heap_block* block, size_t size)
    {
        // Don't split if the remaining space would be too small
        if (block->size < size + MIN_BLOCK_SIZE) {
            return;
        }
        
        // Create new block for the remaining space
        vaddr_t new_block_addr = reinterpret_cast<vaddr_t>(ptr_from(block->data()) + size);
        heap_block* new_block = reinterpret_cast<heap_block*>(new_block_addr);
        
        size_t remaining_size = block->size - size - sizeof(heap_block);
        new (new_block) heap_block(remaining_size, true);
        
        // Update links
        new_block->next = block->next;
        new_block->prev = block;
        if (block->next) {
            block->next->prev = new_block;
        }
        block->next = new_block;
        
        // Update original block
        block->size = size;
        
        total_free_ += sizeof(heap_block); // We added a new header
    }

    void heap::coalesce_block(heap_block* block)
    {
        if (!block->is_free) {
            return;
        }
        
        // Coalesce with next block if it's free and adjacent
        if (block->next && block->next->is_free) {
            uintptr_t block_end = ptr_from(block->data()) + block->size;
            uintptr_t next_start = ptr_from(block->next);
            
            if (block_end == next_start) {
                heap_block* next_block = block->next;
                
                block->size += sizeof(heap_block) + next_block->size;
                block->next = next_block->next;
                if (next_block->next) {
                    next_block->next->prev = block;
                }
                
                total_free_ -= sizeof(heap_block); // We removed a header
            }
        }
        
        // Coalesce with previous block if it's free and adjacent
        if (block->prev && block->prev->is_free) {
            uintptr_t prev_end = ptr_from(block->prev->data()) + block->prev->size;
            uintptr_t block_start = ptr_from(block);
            
            if (prev_end == block_start) {
                heap_block* prev_block = block->prev;
                
                prev_block->size += sizeof(heap_block) + block->size;
                prev_block->next = block->next;
                if (block->next) {
                    block->next->prev = prev_block;
                }
                
                total_free_ -= sizeof(heap_block); // We removed a header
            }
        }
    }

    void* heap::malloc(size_t size)
    {
        if (size == 0) {
            return nullptr;
        }
        
        // Align size to 8-byte boundary
        size = HEAP_ALIGN_UP(size);
        
        // Find a free block
        heap_block* block = find_free_block(size);
        
        // If no block found, try to expand heap
        if (block == nullptr) {
            if (!expand_heap(size + sizeof(heap_block))) {
                lib::log(lib::log_level::CRITICAL, "Out of memory: heap expansion failed");
                return nullptr;
            }
            block = find_free_block(size);
        }
        
        if (block == nullptr) {
            lib::log(lib::log_level::CRITICAL, "Out of memory: no suitable block found");
            return nullptr;
        }
        
        // Split block if it's much larger than needed
        split_block(block, size);
        
        // Mark block as used
        block->is_free = false;
        block->magic = heap_block::MAGIC_USED;
        
        // Update statistics
        total_allocated_ += size;
        total_free_ -= size;
        num_allocations_++;
        
        return block->data();
    }

    void heap::free(void* ptr)
    {
        if (ptr == nullptr) {
            return;
        }
        
        heap_block* block = heap_block::from_data(ptr);
        
        // Validate block
        if (!block->is_valid() || block->is_free) {
            lib::log(lib::log_level::CRITICAL, "Invalid free: corrupted block or double free");
            return;
        }
        
        // Mark as free
        block->is_free = true;
        block->magic = heap_block::MAGIC_FREE;
        
        // Update statistics
        total_allocated_ -= block->size;
        total_free_ += block->size;
        num_frees_++;
        
        // Coalesce with adjacent free blocks
        coalesce_block(block);
    }

    void* heap::realloc(void* ptr, size_t new_size)
    {
        if (ptr == nullptr) {
            return malloc(new_size);
        }
        
        if (new_size == 0) {
            free(ptr);
            return nullptr;
        }
        
        heap_block* block = heap_block::from_data(ptr);
        if (!block->is_valid() || block->is_free) {
            lib::log(lib::log_level::CRITICAL, "Invalid realloc: corrupted block");
            return nullptr;
        }
        
        size_t old_size = block->size;
        new_size = HEAP_ALIGN_UP(new_size);
        
        // If new size fits in current block, we're done
        if (new_size <= old_size) {
            // Optionally split if new size is much smaller
            if (old_size > new_size + MIN_BLOCK_SIZE) {
                split_block(block, new_size);
                total_allocated_ -= (old_size - new_size);
                total_free_ += (old_size - new_size);
            }
            return ptr;
        }
        
        // Need to allocate new block
        void* new_ptr = malloc(new_size);
        if (new_ptr == nullptr) {
            return nullptr;
        }
        
        // Copy old data
        lib::memcpy(new_ptr, ptr, old_size);
        
        // Free old block
        free(ptr);
        
        return new_ptr;
    }

    void* heap::calloc(size_t num, size_t size)
    {
        size_t total_size = num * size;
        
        // Check for overflow
        if (num > 0 && total_size / num != size) {
            return nullptr;
        }
        
        void* ptr = malloc(total_size);
        if (ptr != nullptr) {
            lib::memset(ptr, 0, total_size);
        }
        
        return ptr;
    }

    void* heap::aligned_alloc(size_t alignment, size_t size)
    {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
            // Alignment must be power of 2
            return nullptr;
        }
        
        // We need extra space for alignment + original pointer storage
        size_t total_size = size + alignment + sizeof(void*);
        
        void* raw_ptr = malloc(total_size);
        if (raw_ptr == nullptr) {
            return nullptr;
        }
        
        // Calculate aligned address
        uintptr_t raw_addr = ptr_from(raw_ptr);
        uintptr_t aligned_addr = HEAP_ALIGN_UP(raw_addr + sizeof(void*));
        
        // Store original pointer just before aligned address
        void** orig_ptr_storage = reinterpret_cast<void**>(aligned_addr - sizeof(void*));
        *orig_ptr_storage = raw_ptr;
        
        return reinterpret_cast<void*>(aligned_addr);
    }

    bool heap::validate_heap() const
    {
        heap_block* current = first_block_;
        size_t counted_allocated = 0;
        size_t counted_free = 0;
        
        while (current != nullptr) {
            if (!current->is_valid()) {
                lib::log(lib::log_level::CRITICAL, "Heap corruption: invalid magic in block");
                return false;
            }
            
            if (current->is_free) {
                counted_free += current->size;
            } else {
                counted_allocated += current->size;
            }
            
            current = current->next;
        }
        
        if (counted_allocated != total_allocated_ || counted_free != total_free_) {
            lib::log(lib::log_level::CRITICAL, "Heap corruption: statistics mismatch");
            return false;
        }
        
        return true;
    }

    void heap::print_stats() const
    {
        lib::log(lib::log_level::INFO, "=== Kernel Heap Statistics ===");
        lib::log(lib::log_level::INFO, "Total heap size (KB):");
        lib::log(lib::log_level::INFO, "Allocated (KB):");
        lib::log(lib::log_level::INFO, "Free (KB):");
        lib::log(lib::log_level::INFO, "Total allocations:");
        lib::log(lib::log_level::INFO, "Total frees:");
        lib::log(lib::log_level::INFO, "Outstanding allocations:");
    }

    // Global functions
    void init_kernel_heap(physical* phys, virt* virt_mgr)
    {
        if (g_kernel_heap != nullptr) {
            lib::log(lib::log_level::WARNING, "Kernel heap already initialized");
            return;
        }
        
        // Allocate virtual address space for heap (start after kernel)
        const size_t INITIAL_HEAP_SIZE = 1_MB; // Start with 1MB heap
        
        vaddr_t heap_vaddr = virt_mgr->alloc(INITIAL_HEAP_SIZE);
        if (heap_vaddr == nullptr) {
            lib::log(lib::log_level::CRITICAL, "Failed to allocate virtual memory for kernel heap");
            return;
        }
        
        // Create heap using placement allocation (before heap is ready)
        vaddr_t heap_obj_addr = placement_kalloc(sizeof(heap), true);
        g_kernel_heap = new (heap_obj_addr) heap(phys, virt_mgr, heap_vaddr, INITIAL_HEAP_SIZE);
        
        lib::log(lib::log_level::INFO, "Kernel heap initialized successfully");
    }

    void* kmalloc(size_t size)
    {
        if (g_kernel_heap == nullptr) {
            lib::log(lib::log_level::CRITICAL, "Kernel heap not initialized, using placement allocator");
            return placement_kalloc(size, true);
        }
        
        return g_kernel_heap->malloc(size);
    }

    void kfree(void* ptr)
    {
        if (g_kernel_heap == nullptr || ptr == nullptr) {
            return;
        }
        
        g_kernel_heap->free(ptr);
    }

    void* krealloc(void* ptr, size_t new_size)
    {
        if (g_kernel_heap == nullptr) {
            lib::log(lib::log_level::CRITICAL, "Kernel heap not initialized");
            return nullptr;
        }
        
        return g_kernel_heap->realloc(ptr, new_size);
    }

    void* kcalloc(size_t num, size_t size)
    {
        if (g_kernel_heap == nullptr) {
            lib::log(lib::log_level::CRITICAL, "Kernel heap not initialized");
            return nullptr;
        }
        
        return g_kernel_heap->calloc(num, size);
    }

    void* kmalloc_aligned(size_t alignment, size_t size)
    {
        if (g_kernel_heap == nullptr) {
            lib::log(lib::log_level::CRITICAL, "Kernel heap not initialized");
            return nullptr;
        }
        
        return g_kernel_heap->aligned_alloc(alignment, size);
    }
}