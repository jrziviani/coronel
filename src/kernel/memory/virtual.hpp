#ifndef VIRTUAL_HPP
#define VIRTUAL_HPP

#include "libs/stdint.hpp"
#include "libs/ilist.hpp"
#include "libs/new.hpp"

/*
 * virtual memory
 *
 * Represents the virtual memory. However, unlike physical.h, we will have several
 * "virtual memories". Kernel has a virtual memory as well as each process instance
 * in the system. Thus, an virt object can be copied.
 *
 * Instead of a list of free memory, this abstraction has a list of allocated memory,
 * it means that a new node (16-byte) will be created and appended to the list when
 * handling new allocations.
 */
class virt
{
    struct node
    {
        vaddr_t start;
        size_t size;

        node(vaddr_t st, size_t sz) :
            start(st),
            size(sz)
        {
        }

        uintptr_t addr_to_int() const
        {
            return ptr_from(start);
        }
    };

private:
    lib::ilist<node*> allocated_list_;

public:
    virt();
    virt(const virt &other);

    virt &operator=(const virt &other);

    void setup(paddr_t start, size_t len);

    vaddr_t alloc(size_t size);
    bool alloc(vaddr_t initial_addr, size_t size);
    void free(vaddr_t addr, size_t size);
};

#endif // VIRTUAL_HPP