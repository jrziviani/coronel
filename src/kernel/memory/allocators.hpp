#ifndef ALLOCATORS_HPP
#define ALLOCATORS_HPP

#include "libs/stdint.hpp"

// Placement allocator (early boot, before heap is ready)
vaddr_t placement_kalloc(size_t size, paddr_t *paddr, bool align=false);
vaddr_t placement_kalloc(size_t size, bool align=false);
void kfree_block(size_t size);
uintptr_t current();

// Kernel heap allocator (after memory management is initialized)
vaddr_t kalloc(size_t size);
void kfree(vaddr_t addr);

// Additional heap functions (these use the heap directly)
namespace memory {
    void* kmalloc(size_t size);
    void kfree(void* ptr);
    void* krealloc(void* ptr, size_t new_size);
    void* kcalloc(size_t num, size_t size);
    void* kmalloc_aligned(size_t alignment, size_t size);
}

#endif // ALLOCATORS_HPP