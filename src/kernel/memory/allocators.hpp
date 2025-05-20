#ifndef ALLOCATORS_HPP
#define ALLOCATORS_HPP

#include "libs/stdint.hpp"

vaddr_t placement_kalloc(size_t size, paddr_t *paddr, bool align=false);
vaddr_t placement_kalloc(size_t size, bool align=false);
void kfree_block(size_t size);

vaddr_t kalloc(size_t size);
void kfree(vaddr_t addr);

uintptr_t current();

#endif // ALLOCATORS_HPP