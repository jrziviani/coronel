#ifndef PAGING_H
#define PAGING_H

#include "pagetable.h"

class paging
{
    uintptr_t *kernel_pages_;
    pml4_t *kernel_directory_;

    private:
    pte_t *get_page(paddr_t top_dir, vaddr_t vaddr, bool make);

    public:
    paging() = default;
    ~paging() = default;

    int map(paddr_t top_dir, vaddr_t vaddr, paddr_t paddr, uint8_t flags);
    int map(vaddr_t vaddr, paddr_t paddr, uint8_t flags);

    void unmap(paddr_t top_dir, vaddr_t vaddr);
    void unmap(vaddr_t vaddr);

    vaddr_t mapio(uintptr_t addr, uint8_t flags);
    void unmapio(vaddr_t vaddr);

    paddr_t create_top_page_directory();
}; 

#endif // PAGING_H