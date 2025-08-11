#ifndef HEAP_HPP
#define HEAP_HPP

#include "libs/stdint.hpp"
#include "physical.hpp"
#include "virtual.hpp"

/*
 * Kernel Heap Allocator
 * 
 * This is a proper kernel heap implementation that uses both physical
 * and virtual memory managers. It provides malloc/free style allocation
 * with proper alignment and coalescing.
 *
 * The heap manages a region of virtual memory and maps physical frames
 * to it on demand. It uses a simple linked list of free blocks with
 * coalescing to reduce fragmentation.
 */

namespace memory
{
    // Forward declarations
    class heap;

    // Global heap instance
    extern heap* g_kernel_heap;

    // Heap block header - every allocation has this header
    struct heap_block
    {
        size_t size;           // Size of the data portion (not including header)
        bool is_free;          // Is this block free?
        heap_block* next;      // Next block in the list
        heap_block* prev;      // Previous block in the list
        
        // Magic values for corruption detection
        static constexpr uint32_t MAGIC_FREE = 0xDEADBEEF;
        static constexpr uint32_t MAGIC_USED = 0xCAFEBABE;
        uint32_t magic;
        
        heap_block(size_t sz, bool free) : 
            size(sz), 
            is_free(free), 
            next(nullptr), 
            prev(nullptr),
            magic(free ? MAGIC_FREE : MAGIC_USED) {}
            
        void* data() { 
            return reinterpret_cast<void*>(this + 1); 
        }
        
        static heap_block* from_data(void* ptr) {
            return reinterpret_cast<heap_block*>(ptr) - 1;
        }
        
        bool is_valid() const {
            return magic == MAGIC_FREE || magic == MAGIC_USED;
        }
    };

    class heap
    {
    private:
        physical* phys_manager_;
        virt* virt_manager_;
        
        vaddr_t heap_start_;
        vaddr_t heap_end_;
        size_t heap_size_;
        
        heap_block* first_block_;
        
        // Statistics
        size_t total_allocated_;
        size_t total_free_;
        size_t num_allocations_;
        size_t num_frees_;
        
        // Minimum allocation size (including header)
        static constexpr size_t MIN_BLOCK_SIZE = sizeof(heap_block) + 16;
        
        // Expand heap when needed
        bool expand_heap(size_t min_size);
        
        // Split a block if it's large enough
        void split_block(heap_block* block, size_t size);
        
        // Coalesce adjacent free blocks
        void coalesce_block(heap_block* block);
        
        // Find a free block that fits the size (first-fit)
        heap_block* find_free_block(size_t size);
        
        // Map physical memory to virtual addresses
        bool map_pages(vaddr_t start, size_t size);
        
        // Unmap virtual addresses
        void unmap_pages(vaddr_t start, size_t size);

    public:
        heap(physical* phys, virt* virt_mgr, vaddr_t start, size_t initial_size);
        ~heap();
        
        // Main allocation functions
        void* malloc(size_t size);
        void free(void* ptr);
        void* realloc(void* ptr, size_t new_size);
        void* calloc(size_t num, size_t size);
        
        // Alignment-aware allocation
        void* aligned_alloc(size_t alignment, size_t size);
        
        // Statistics and debugging
        void print_stats() const;
        void dump_blocks() const;
        bool validate_heap() const;
        
        // Heap management
        size_t get_total_size() const { return heap_size_; }
        size_t get_allocated_size() const { return total_allocated_; }
        size_t get_free_size() const { return total_free_; }
        
        // Disable copy/move
        heap(const heap&) = delete;
        heap(heap&&) = delete;
        heap& operator=(const heap&) = delete;
        heap& operator=(heap&&) = delete;
    };

    // Global functions for kernel heap
    void init_kernel_heap(physical* phys, virt* virt_mgr);
    void* kmalloc(size_t size);
    void kfree(void* ptr);
    void* krealloc(void* ptr, size_t new_size);
    void* kcalloc(size_t num, size_t size);
    void* kmalloc_aligned(size_t alignment, size_t size);
}

#endif // HEAP_HPP