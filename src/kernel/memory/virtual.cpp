#include "allocators.hpp"
#include "virtual.hpp"
#include "config.hpp"

const vaddr_t VADDR_START   = reinterpret_cast<vaddr_t>(0);
const size_t VADDR_SIZE     = 256_GB;

virt::virt()
{
    // Create initial free region representing the entire virtual address space
    vaddr_t place = placement_kalloc(sizeof(node));
    node *initial_free = new (place) node(VADDR_START, VADDR_SIZE);
    free_mem_list_.push_back(initial_free);
}

virt::virt(const virt &other)
{
    free_mem_list_.clear();
    for (auto slot : other.free_mem_list_) {
        vaddr_t place = placement_kalloc(sizeof(node));
        node *new_node = new (place) node(slot->start, slot->size);
        free_mem_list_.push_back(new_node);
    }
}

virt &virt::operator=(const virt &other)
{
    if (this != &other) {
        free_mem_list_.clear();
        for (auto slot : other.free_mem_list_) {
            vaddr_t place = placement_kalloc(sizeof(node));
            node *new_node = new (place) node(slot->start, slot->size);
            free_mem_list_.push_back(new_node);
        }
    }

    return *this;
}

vaddr_t virt::alloc(size_t size)
{
    // Align size to frame boundary
    size_t aligned_size = ALIGN_UP(size);
    
    for (auto free_spot : free_mem_list_) {
        // Skip zero-sized regions (marked for deletion) and too-small regions
        if (free_spot->size == 0 || free_spot->size < aligned_size) {
            continue;
        }
        
        // Allocate from the beginning of the free region (first-fit)
        vaddr_t allocated_addr = free_spot->start;
        
        // If this allocation uses the entire free region, mark it as used
        if (free_spot->size == aligned_size) {
            // Mark as used (since we can't remove from ilist)
            free_spot->size = 0;
        } else {
            // Shrink the free region
            free_spot->start = reinterpret_cast<vaddr_t>(ptr_from(free_spot->start) + aligned_size);
            free_spot->size -= aligned_size;
        }
        
        return allocated_addr;
    }

    return nullptr;
}

/*
    [free_spot                               ]--> null
    [free_spot      ]+------------>[new slot ]--> null
                     |
                     +---> initial_addr + size
*/
bool virt::alloc(vaddr_t initial_addr, size_t size)
{
    uintptr_t addr = reinterpret_cast<uintptr_t>(initial_addr);
    for (auto free_spot : free_mem_list_) {
        // search for the slot which contains the address we're looking for
        uintptr_t free_addr = free_spot->addr_to_int();
        if (free_addr < addr && free_addr + free_spot->size <= addr + size) {
            continue;
        }

        // create a new slot after the initial_addr + size
        vaddr_t place = placement_kalloc(sizeof(node));
        node *slot = new (place) node(reinterpret_cast<vaddr_t>(addr + size),
                                      free_spot->size);
        free_mem_list_.push_back(slot);

        // resize the current slot
        free_spot->size = addr - free_addr;

        return true;
    }

    return false;
}

void virt::free(vaddr_t addr, size_t size)
{
    size_t aligned_size = ALIGN_UP(size);
    uintptr_t free_start = ptr_from(addr);
    uintptr_t free_end = free_start + aligned_size;
    
    // Try to coalesce with existing free regions
    bool merged = false;
    for (auto current : free_mem_list_) {
        // Skip zero-sized regions (marked for deletion)
        if (current->size == 0) {
            continue;
        }
        
        uintptr_t current_start = ptr_from(current->start);
        uintptr_t current_end = current_start + current->size;
        
        // Check if we can merge with this free region
        if (free_end == current_start) {
            // Merge: our freed region is right before this free region
            current->start = addr;
            current->size += aligned_size;
            merged = true;
            break;
        } else if (current_end == free_start) {
            // Merge: our freed region is right after this free region
            current->size += aligned_size;
            
            // Check if we can merge with another region after this one
            for (auto next : free_mem_list_) {
                if (next == current || next->size == 0) {
                    continue;
                }
                uintptr_t next_start = ptr_from(next->start);
                if (current_start + current->size == next_start) {
                    // Triple merge: extend current to include next
                    current->size += next->size;
                    // Mark for deletion (since we can't erase)
                    next->size = 0;
                    break;
                }
            }
            merged = true;
            break;
        }
    }
    
    // No coalescing possible, create new free region
    if (!merged) {
        vaddr_t place = placement_kalloc(sizeof(node));
        node *new_free = new (place) node(addr, aligned_size);
        free_mem_list_.push_back(new_free);
    }
}