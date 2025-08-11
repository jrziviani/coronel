#ifndef USER_ALLOCATOR_HPP
#define USER_ALLOCATOR_HPP

/**
 * User Space Memory Allocator
 * 
 * This implements a user space heap allocator that follows standard OS conventions.
 * Each process gets its own isolated heap with process-specific page directory.
 * 
 * Memory Layout (following ELF/POSIX standards):
 * 
 * 0x00000000 - 0x003FFFFF: NULL Guard (4MB) - Unmapped for NULL pointer protection
 * 0x00400000 - 0x07FFFFFF: Code + Data segments (128MB max)
 * 0x08000000 - 0x3FFFFFFF: Process Heap (768MB max) - Managed by this allocator  
 * 0x40000000 - 0x6FFFFFFF: Shared libraries/mmap (768MB)
 * 0x70000000 - 0x7FFF0000: Reserved
 * 0x7FFF0000 - 0x80000000: User Stack (8MB default, grows down)
 * 0x80000000+            : Kernel space (high memory)
 * 
 */

#include "libs/stdint.hpp"
#include "memory/physical.hpp"
#include "memory/virtual.hpp"

/*
 * User Space Memory Allocator
 * 
 * This allocator is designed for user mode programs. Unlike the kernel heap,
 * each process gets its own instance of this allocator. The allocator:
 * 
 * 1. Operates in user virtual address space (0x0 - 0x7FFFFFFF)
 * 2. Uses system calls to request/release memory from kernel
 * 3. Provides malloc/free interface for user programs
 * 4. Supports process isolation and memory protection
 */

namespace memory
{
    // Forward declarations
    class process;

    // Standard user space memory layout (following ELF conventions)
    constexpr uintptr_t USER_NULL_GUARD_SIZE  = 0x400000;     // 4MB NULL guard (0x0 - 0x3FFFFF)
    constexpr uintptr_t USER_CODE_START       = 0x400000;     // 4MB - standard ELF start
    constexpr uintptr_t USER_DATA_MAX          = 0x08000000;   // 128MB max for code+data
    constexpr uintptr_t USER_HEAP_START_ADDR   = 0x08000000;   // 128MB - heap start
    constexpr uintptr_t USER_HEAP_MAX_ADDR     = 0x40000000;   // 1GB - heap limit
    constexpr uintptr_t USER_MMAP_START        = 0x40000000;   // 1GB - shared libs/mmap
    constexpr uintptr_t USER_MMAP_MAX          = 0x70000000;   // 1.75GB - mmap limit
    constexpr uintptr_t USER_STACK_TOP         = 0x7FFF0000;   // Near 2GB - stack top
    constexpr size_t    USER_STACK_SIZE        = 8_MB;         // 8MB default stack
    constexpr size_t    USER_HEAP_INITIAL_SIZE = 1_MB;         // 1MB initial heap

    // Helper functions to get vaddr_t from constants
    inline vaddr_t get_user_code_start() { return reinterpret_cast<vaddr_t>(USER_CODE_START); }
    inline vaddr_t get_user_heap_start() { return reinterpret_cast<vaddr_t>(USER_HEAP_START_ADDR); }
    inline vaddr_t get_user_heap_max() { return reinterpret_cast<vaddr_t>(USER_HEAP_MAX_ADDR); }
    inline vaddr_t get_user_mmap_start() { return reinterpret_cast<vaddr_t>(USER_MMAP_START); }
    inline vaddr_t get_user_stack_top() { return reinterpret_cast<vaddr_t>(USER_STACK_TOP); }

    // User allocation block header (optimized for space efficiency)
    struct user_block
    {
        uint32_t size;        // Size of data portion (32-bit limits individual blocks to 2GB)
        uint32_t flags;       // Flags: bit 0 = is_free, bits 24-31 = magic for corruption detection
        user_block* next;     // Next block (64-bit pointers)
        user_block* prev;     // Previous block (64-bit pointers)
        
        // Note: 32-bit size field limits individual allocations to 2GB but saves space
        // in block headers. For larger allocations, use memory mapping (mmap) instead.
        
        static constexpr uint32_t FLAG_FREE = 0x01;
        static constexpr uint32_t MAGIC_MASK = 0xFF000000;
        static constexpr uint32_t MAGIC_FREE = 0xAA000000;
        static constexpr uint32_t MAGIC_USED = 0x55000000;
        
        user_block(uint32_t sz, bool free) : 
            size(sz), 
            flags(free ? FLAG_FREE : 0),
            next(nullptr), 
            prev(nullptr) {
            flags |= free ? MAGIC_FREE : MAGIC_USED;
        }
        
        bool is_free() const { return flags & FLAG_FREE; }
        void set_free(bool free) { 
            flags = (flags & ~(FLAG_FREE | MAGIC_MASK)) | (free ? FLAG_FREE : 0);
            flags |= free ? MAGIC_FREE : MAGIC_USED;
        }
        
        bool is_valid() const {
            uint32_t magic = flags & MAGIC_MASK;
            return magic == MAGIC_FREE || magic == MAGIC_USED;
        }
        
        void* data() { return reinterpret_cast<void*>(this + 1); }
        
        static user_block* from_data(void* ptr) {
            return reinterpret_cast<user_block*>(ptr) - 1;
        }
    };

    class user_allocator
    {
    private:
        vaddr_t heap_start_;
        vaddr_t heap_current_;
        vaddr_t heap_limit_;
        
        user_block* first_block_;
        
        // Process context (for system calls)
        paddr_t page_directory_;
        
        // Statistics (kept small for user space)
        uint32_t total_allocated_;
        uint32_t num_blocks_;
        
        // Internal methods
        user_block* find_free_block(uint32_t size);
        bool expand_heap(uint32_t min_size);
        void split_block(user_block* block, uint32_t size);
        void coalesce_block(user_block* block);
        
        // System call interface
        bool sys_request_memory(vaddr_t addr, size_t size);
        void sys_release_memory(vaddr_t addr, size_t size);

    public:
        user_allocator(paddr_t page_dir);
        ~user_allocator();
        
        // Standard allocation interface
        void* malloc(size_t size);
        void free(void* ptr);
        void* realloc(void* ptr, size_t new_size);
        void* calloc(size_t num, size_t size);
        
        // User-specific features
        bool set_heap_limit(size_t max_size);
        size_t get_heap_size() const;
        bool validate_heap() const;
        
        // Process management
        void cleanup_on_exit();
        
        // Disable copy/move
        user_allocator(const user_allocator&) = delete;
        user_allocator(user_allocator&&) = delete;
        user_allocator& operator=(const user_allocator&) = delete;
        user_allocator& operator=(user_allocator&&) = delete;
    };

    // Process memory management
    class process_memory
    {
    private:
        paddr_t page_directory_;
        virt virtual_manager_;
        user_allocator* heap_;
        
        // Memory regions
        vaddr_t stack_top_;
        size_t stack_size_;
        vaddr_t code_start_;
        size_t code_size_;
        vaddr_t data_start_;
        size_t data_size_;
        
    public:
        process_memory(paddr_t page_dir);
        ~process_memory();
        
        // Setup process memory layout
        bool setup_memory_layout(vaddr_t code_addr, size_t code_size,
                            vaddr_t data_addr, size_t data_size);
        
        // Heap management
        user_allocator* get_heap() { return heap_; }
        
        // Stack management
        bool setup_stack(size_t stack_size);
        vaddr_t get_stack_top() const { return stack_top_; }
        
        // Memory mapping for system calls
        bool map_user_page(vaddr_t vaddr, uint8_t permissions);
        void unmap_user_page(vaddr_t vaddr);
        
        // Process cleanup
        void cleanup_all();
        
        // Validation
        bool validate_user_pointer(void* ptr, size_t size) const;
    };

    // System call interface for user programs
    namespace syscalls {
        // Memory system calls that user programs can invoke
        extern "C" {
            void* sys_malloc(size_t size);
            void sys_free(void* ptr);
            void* sys_realloc(void* ptr, size_t new_size);
            void* sys_calloc(size_t num, size_t size);
            int sys_brk(void* addr);        // Traditional Unix brk() system call
            void* sys_mmap(void* addr, size_t length, int prot, int flags);
            int sys_munmap(void* addr, size_t length);
        }
    }
}

#endif // USER_ALLOCATOR_HPP