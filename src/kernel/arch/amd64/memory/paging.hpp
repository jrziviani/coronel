#ifndef PAGING_HPP
#define PAGING_HPP

#include "pagetable.hpp"

class paging
{
    uintptr_t *kernel_pages_;
    pml4_t *kernel_directory_;

    private:
    pte_t *get_page(paddr_t page_dir, vaddr_t vaddr, uint8_t flags, bool make);

    public:
    paging() = default;
    ~paging() = default;

    int map(paddr_t page_dir, vaddr_t vaddr, paddr_t paddr, uint8_t flags);
    int map(vaddr_t vaddr, paddr_t paddr, uint8_t flags);

    void unmap(paddr_t page_dir, vaddr_t vaddr);
    void unmap(vaddr_t vaddr);

    vaddr_t mapio(uintptr_t addr, uint8_t flags);
    void unmapio(vaddr_t vaddr);

    paddr_t create_page_directory();
    
    // User space
    int map_user(paddr_t page_dir, uint64_t vaddr, uint64_t paddr);
    bool setup_user_memory_layout(paddr_t page_dir);
    
    paddr_t create_user_page_directory();
}; 

#endif // PAGING_HPP