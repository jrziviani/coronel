#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include "libs/stdint.hpp"
#include "libs/multiboot.hpp"
#include "physical.hpp"
#include "virtual.hpp"
#include "heap.hpp"

namespace memory
{
    // Global memory managers
    extern physical* g_physical_manager;
    extern virt* g_kernel_virtual_manager;

    // Initialize memory management from multiboot information
    void initialize_memory(multiboot_info_t* bootinfo);

    // Print memory information
    void print_memory_info();

    // Get memory statistics
    struct memory_stats
    {
        size_t total_physical;
        size_t available_physical;
        size_t used_physical;
        size_t kernel_heap_size;
        size_t kernel_heap_used;
    };

    memory_stats get_memory_stats();
}

#endif // MEMORY_MANAGER_HPP
