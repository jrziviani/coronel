#include "memory_manager.hpp"
#include "allocators.hpp"
#include "config.hpp"
#include "libs/logger.hpp"

namespace memory {

    // Global memory managers
    physical* g_physical_manager = nullptr;
    virt* g_kernel_virtual_manager = nullptr;

    void initialize_memory(multiboot_info_t* bootinfo)
    {
        lib::log(lib::log_level::INFO, "Initializing memory management...");
        
        // Parse memory map from multiboot
        size_t total_memory = 0;
        paddr_t largest_free_start = nullptr;
        size_t largest_free_size = 0;
        
        if (bootinfo->flags & MULTIBOOT_INFO_MEM_MAP) {
            lib::log(lib::log_level::INFO, "Processing memory map...");
            
            multiboot_memory_map_t* mmap = reinterpret_cast<multiboot_memory_map_t*>(
                bootinfo->mmap_addr + KVIRTUAL_ADDRESS);
            multiboot_memory_map_t* mmap_end = reinterpret_cast<multiboot_memory_map_t*>(
                bootinfo->mmap_addr + bootinfo->mmap_length + KVIRTUAL_ADDRESS);
            
            while (mmap < mmap_end) {
                lib::log(lib::log_level::INFO, "Processing memory region");
                
                if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                    total_memory += mmap->len;
                    
                    // Skip memory used by kernel (first 8MB)
                    if (mmap->addr < 8_MB) {
                        if (mmap->addr + mmap->len > 8_MB) {
                            // Memory region extends beyond kernel
                            uint64_t available_start = 8_MB;
                            uint64_t available_size = (mmap->addr + mmap->len) - 8_MB;
                            
                            if (available_size > largest_free_size) {
                                largest_free_start = ptr_to<paddr_t>(available_start);
                                largest_free_size = available_size;
                            }
                        }
                    } else {
                        // Memory region is entirely above kernel
                        if (mmap->len > largest_free_size) {
                            largest_free_start = ptr_to<paddr_t>(mmap->addr);
                            largest_free_size = mmap->len;
                        }
                    }
                }
                
                // Move to next entry
                mmap = reinterpret_cast<multiboot_memory_map_t*>(
                    reinterpret_cast<uintptr_t>(mmap) + mmap->size + sizeof(mmap->size));
            }
        } else if (bootinfo->flags & MULTIBOOT_INFO_MEMORY) {
            // Fallback to basic memory info
            lib::log(lib::log_level::INFO, "Using basic memory info (no memory map)");
            
            // Convert KB to bytes
            size_t lower_mem = bootinfo->mem_lower * 1024; 
            size_t upper_mem = bootinfo->mem_upper * 1024;
            
            total_memory = lower_mem + upper_mem;
            
            // Assume memory starts at 8MB (after kernel)
            largest_free_start = ptr_to<paddr_t>(8_MB);
            largest_free_size = upper_mem - (8_MB - 1_MB); // Upper memory starts at 1MB
        } else {
            lib::log(lib::log_level::CRITICAL, "No memory information available from bootloader!");
            return;
        }
        
        lib::log(lib::log_level::INFO, "Total memory calculated");
        lib::log(lib::log_level::INFO, "Largest free region found");
        
        if (largest_free_size < 4_MB) {
            lib::log(lib::log_level::CRITICAL, "Not enough free memory to initialize managers");
            return;
        }
        
        // Initialize physical memory manager
        vaddr_t phys_mgr_addr = placement_kalloc(sizeof(physical), true);
        g_physical_manager = new (phys_mgr_addr) physical();
        g_physical_manager->setup(largest_free_start, largest_free_size);
        
        lib::log(lib::log_level::INFO, "Physical memory manager initialized");
        
        // Initialize kernel virtual memory manager
        vaddr_t virt_mgr_addr = placement_kalloc(sizeof(virt), true);
        g_kernel_virtual_manager = new (virt_mgr_addr) virt();
        
        lib::log(lib::log_level::INFO, "Virtual memory manager initialized");
        
        // Initialize kernel heap
        init_kernel_heap(g_physical_manager, g_kernel_virtual_manager);
        
        lib::log(lib::log_level::INFO, "Memory management initialization complete");
    }

    void print_memory_info()
    {
        if (g_physical_manager == nullptr || g_kernel_virtual_manager == nullptr) {
            lib::log(lib::log_level::WARNING, "Memory managers not initialized");
            return;
        }
        
        lib::log(lib::log_level::INFO, "=== Memory Information ===");
        
        if (g_kernel_heap != nullptr) {
            g_kernel_heap->print_stats();
        }
        
        // TODO: Add physical manager statistics when available
    }

    memory_stats get_memory_stats()
    {
        memory_stats stats = {};
        
        if (g_kernel_heap != nullptr) {
            stats.kernel_heap_size = g_kernel_heap->get_total_size();
            stats.kernel_heap_used = g_kernel_heap->get_allocated_size();
        }
        
        // TODO: Add physical memory statistics when available
        
        return stats;
    }

}