#include "paging.hpp"

#include "config.hpp"
#include "libs/logger.hpp"
#include "libs/string.hpp"
#include "memory/allocators.hpp"
#include "arch/amd64/instructions.hpp"

#define PTE(addr)       ((ptr_from(addr) >> 12) & 0x1ff)
#define PDE(addr)       ((ptr_from(addr) >> 21) & 0x1ff)
#define PDPT(addr)      ((ptr_from(addr) >> 30) & 0x1ff)
#define PML4(addr)      ((ptr_from(addr) >> 39) & 0x1ff)

#define ADDRESS(addr)   ((ptr_from(addr) & ~0xfff) + KVIRTUAL_ADDRESS)
#define PRESENT(addr)   ((ptr_from(addr) & 0x1) == 1) 

// User space memory layout constants
constexpr uint64_t USER_SPACE_START = 0x400000UL;     // 4MB - start of user space
constexpr uint64_t USER_SPACE_END = 0x800000UL;       // 8MB - end of user space
constexpr uint64_t USER_STACK_TOP = 0x7FFFFF0UL;      // Top of user stack
constexpr uint64_t USER_STACK_SIZE = 0x10000UL;       // 64KB user stack

enum PERMISSION_FLAGS {
    PRESENT = 0x01,
    WRITABLE = 0x02,
    USER = 0x04,
    USER_RW = 0x07
};

/*
 *   ADDRESS    CONTENT     page_dir = 0x1000
 *   0x1000     0x8003
 *   0x1008     0x9003
 *   0x1010     0xA003
 *   0x1018     0xB003
 *   0x1020     0xC003
 *   ...        ...
 *
 *   page_dir contains the address to the first element of PML4 table (in other words, page_dir
 *   points to PML4), and the content of each PML4 entry (8 bytes each entry in this 64-bit
 *   world) is the address to the first PDPT element.
 *
 *   The scheme continues:
 *   ADDRESS    CONTENT
 *   0x8000     0x10003
 *   0x8008     0x11003
 *   0x8010     0x12003
 *   ...        ...
 *
 *   which can be represented as (in practice, sum 0x3 to all entries):
 *                PML4           PDPT           PDE             PTE              FRAME
 *   page_dir --> 0x1000 ------> 0x8000 ------> 0x10000 ------> 0x100000 ------> 0x200000
 *                0x1008 ---+    0x8008 ---+    0x11000 ...     0x101000 ...     ...
 *                0x1010 -+ |    0x8010    |    0x12000         0x102000
 *                ...     | |    ...      ...   ...             ...
 *                        | +--> 0x9000 ------> 0xN0000
 *                        |      0x9008 ---+    0xN1000
 *                       ...     0x9010    |    0xN2000
 *                               ...      ...   ...
 */ 
pte_t *paging::get_page(paddr_t page_dir, vaddr_t vaddr, uint8_t flags, bool make)
{
    auto pml4 = PML4(vaddr);
    auto pdpt = PDPT(vaddr);
    auto pde  = PDE(vaddr);

    uint64_t dir_flags = 0x01 | 0x02;  // Present + Writable (for kernel access)
    if (flags & 0x04) {  // If user page, make directories user-accessible too
        dir_flags |= 0x04;
    }

    // page_dir points to PML4 table and PML4(vaddr) is the index, this cell points to the PDPT
    // table. this code checks if the PDPT table at PML4[index] is present, if it's not present
    // (and the caller wants it present) we allocate a new PDPT table and store its (physical)
    // addresss at PML4[index]. If the page is not present and the caller doesn't want to make
    // it present, simply return null.
    //
    // it's important to notice that we'll store the physical address but the table itself is
    // managed using the virtual address. thanks to identity mapping, we can be sure that:
    //     virtual address = physical address & ~0xfff + KVIRTUAL_ADDRESS
    pml4_t *pml4_table = reinterpret_cast<pml4_t*>(ADDRESS(page_dir));
    if (!PRESENT(pml4_table->dirs[pml4]) && make) {
        paddr_t paddr;
        pdpt_t *tmp = reinterpret_cast<pdpt_t*>(placement_kalloc(sizeof(pdpt_t), &paddr, true));
        lib::memset(tmp, 0, sizeof(pdpt_t));
        pml4_table->dirs[pml4] = ptr_from(paddr) | dir_flags;
    }
    else if (!PRESENT(pml4_table->dirs[pml4])) {
        return nullptr;
    }

    // PML4[index] points to a PDPT table and PDPT(vaddr) is the index, this
    // PDPT[index] points to the PDE table and the process is exactly the same
    // used by PML4 above.
    pdpt_t *pdpt_table = reinterpret_cast<pdpt_t*>(ADDRESS(pml4_table->dirs[pml4]));
    if (!PRESENT(pdpt_table->dirs[pdpt]) && make) {
        paddr_t paddr;
        pde_t *tmp = reinterpret_cast<pde_t*>(placement_kalloc(sizeof(pde_t), &paddr, true));
        lib::memset(tmp, 0, sizeof(pde_t));
        pdpt_table->dirs[pdpt] = ptr_from(paddr) | dir_flags;
    }
    else if (!PRESENT(pdpt_table->dirs[pdpt])) {
        return nullptr;
    }

    // PDPT[index] points to a PDE table and PDE(vaddr) is the index, this PDE[index]
    // points to the PTE table.
    pde_t *pde_table = reinterpret_cast<pde_t*>(ADDRESS(pdpt_table->dirs[pdpt]));
    if (!PRESENT(pde_table->dirs[pde]) && make) {
        paddr_t paddr;
        pte_t *tmp = reinterpret_cast<pte_t*>(placement_kalloc(sizeof(pte_t), &paddr, true));
        lib::memset(tmp, 0, sizeof(pte_t));
        pde_table->dirs[pde] = ptr_from(paddr) | dir_flags;
    }
    else if (!PRESENT(pde_table->dirs[pde])) {
        return nullptr;
    }

    // returns a pointer to PTE table
    return reinterpret_cast<pte_t*>(ADDRESS(pde_table->dirs[pde]));
}

void paging::unmap(paddr_t page_dir, vaddr_t vaddr)
{
    auto addr = ptr_from(vaddr);
    auto pml4 = PML4(addr);
    auto pdpt = PDPT(addr);
    auto pde  = PDE(addr);
    auto pte  = PTE(addr);

    pml4_t *pml4_table = reinterpret_cast<pml4_t*>(ADDRESS(page_dir));
    if (!PRESENT(pml4_table->dirs[pml4])) {
        lib::log(lib::log_level::CRITICAL, "Unmap: pml4 not present");
        return;
    }

    pdpt_t *pdpt_table = reinterpret_cast<pdpt_t*>(ADDRESS(pml4_table->dirs[pml4]));
    if (!PRESENT(pdpt_table->dirs[pdpt])) {
        lib::log(lib::log_level::CRITICAL, "Unmap: pdpt not present");
        return;
    }

    pde_t *pde_table = reinterpret_cast<pde_t*>(ADDRESS(pdpt_table->dirs[pdpt]));
    if (!PRESENT(pde_table->dirs[pde])) {
        lib::log(lib::log_level::CRITICAL, "Unmap: pde not present");
        return;
    }

    pte_t *entry = reinterpret_cast<pte_t*>(ADDRESS(pde_table->dirs[pde]));
    if (!PRESENT(entry->pages[pte])) {
        lib::log(lib::log_level::CRITICAL, "Unmap: pte not present");
        return;
    }

    entry->pages[pte] &= ~0xfff;
    insn::tlb_flush(0);
}

void paging::unmap(vaddr_t vaddr)
{
    unmap(insn::get_current_page(), vaddr);
}

int paging::map(paddr_t dir, vaddr_t vaddr, paddr_t paddr, uint8_t flags)
{
    auto pte  = PTE(ptr_from(vaddr));

    pte_t *page = get_page(dir, vaddr, flags, true);
    if (page == nullptr) {
        // Failed to get page for mapping
        return -1;
    }
    
    // Build the page table entry with proper flags
    uint64_t entry = ptr_from(paddr) & ~0xFFF;  // Physical address (page aligned)
    
    if (flags & PERMISSION_FLAGS::PRESENT) entry |= PERMISSION_FLAGS::PRESENT;
    if (flags & PERMISSION_FLAGS::WRITABLE) entry |= PERMISSION_FLAGS::WRITABLE;
    if (flags & PERMISSION_FLAGS::USER) entry |= PERMISSION_FLAGS::USER;

    page->pages[pte] = entry;

    insn::tlb_flush(dir);

    return 0;
}

int paging::map(vaddr_t vaddr, paddr_t paddr, uint8_t flags)
{
    return map(insn::get_current_page(), vaddr, paddr, flags);
}

vaddr_t paging::mapio(uintptr_t addr, uint8_t flags)
{
    (void)flags;

    uintptr_t offset = addr & ~0xfff0'0000;
    vaddr_t vaddr = ptr_to<vaddr_t>(PCI_VIRTUAL_ADDRESS + offset);
    auto pte = PTE(PCI_VIRTUAL_ADDRESS | offset);

    pte_t *page = get_page(insn::get_current_page(), vaddr, 0x0, true);
    page->pages[pte] = addr + 0x3;

    return vaddr;
}

void paging::unmapio(vaddr_t vaddr)
{
    unmap(vaddr);
}

paddr_t paging::create_page_directory()
{
    // creates a new map with kernel code already mapped into it
    auto pml4_index = PML4(KVIRTUAL_ADDRESS);
    paddr_t page_dir;
    pml4_t *page_dir_virt = static_cast<pml4_t*>(placement_kalloc(sizeof(pml4_t), &page_dir, true));
    lib::memset(page_dir_virt, 0x0, sizeof(pml4_t));

    pml4_t *pml4_table = ptr_to<pml4_t*>(ADDRESS(insn::get_current_page()));
    if (!PRESENT(pml4_table->dirs[pml4_index])) {
        lib::log(lib::log_level::CRITICAL, "Create top dir pagetable");
        return nullptr;
    }

    page_dir_virt->dirs[pml4_index] = pml4_table->dirs[pml4_index];

    return page_dir;
}

int paging::map_user(paddr_t page_dir, uint64_t vaddr, uint64_t paddr)
{
    // Map a single page with user permissions
    return map(page_dir, ptr_to<vaddr_t>(vaddr), ptr_to<paddr_t>(paddr), PERMISSION_FLAGS::USER_RW);
}

paddr_t paging::create_user_page_directory()
{
    // Create a new page directory that includes kernel mappings
    paddr_t user_page_dir = create_page_directory();
    if (user_page_dir == nullptr) {
        // Failed to create user page directory
        return nullptr;
    }
    
    return user_page_dir;
}

bool paging::setup_user_memory_layout(paddr_t page_dir)
{
    // Set up user stack - allocate physical pages for user stack
    uint64_t stack_bottom = USER_STACK_TOP - USER_STACK_SIZE;
    size_t stack_pages = USER_STACK_SIZE / 0x1000;
    
    for (size_t i = 0; i < stack_pages; i++) {
        // Allocate physical memory for stack page
        paddr_t stack_phys;
        void* stack_mem = placement_kalloc(0x1000, &stack_phys, true);
        if (stack_mem == nullptr) {
            // Failed to allocate stack memory
            return false;
        }
        
        // Clear the stack memory
        lib::memset(stack_mem, 0, 0x1000);
        
        // Map stack page with user permissions
        uint64_t virtual_addr = stack_bottom + (i * 0x1000);
        if (map_user(page_dir, virtual_addr, ptr_from(stack_phys)) != 0) {
            // Failed to map user stack page
            return false;
        }
    }
    
    return true;
}