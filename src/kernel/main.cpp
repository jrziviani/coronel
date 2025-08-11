#include "libs/multiboot.hpp"
#include "drivers/bus/pci.hpp"
#include "memory/memory_manager.hpp"

#include "archs.hpp"
#include "config.hpp"
#include "drivers/peripherals/keyboard.hpp"


void force_pic_mode() {
    // Disable APIC if present to force PIC mode
    // Check if APIC is enabled via MSR
    uint32_t eax, edx;
    asm volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0x1B));  // IA32_APIC_BASE MSR
    
    if (eax & 0x800) {  // APIC Global Enable bit
        // Disable APIC to force PIC mode
        eax &= ~0x800;
        asm volatile("wrmsr" : : "a"(eax), "d"(edx), "c"(0x1B));
    }
}

void kmain(multiboot_info_t *bootinfo, unsigned long magic)
{
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        return;
    }

    auto *arch = archs::get_arch();
    if (arch == nullptr) {
        return;
    }

    auto *video = arch->get_video();
    if (video == nullptr) {
        return;
    }

    video->clear();
    video->print("Welcome to CoronelOS!\nArch: ", archs::get_arch_name(), '\n');

    // Initialize memory management early
    memory::initialize_memory(bootinfo);

    if (bootinfo->flags & MULTIBOOT_INFO_CMDLINE) {
        uintptr_t cmdline = bootinfo->cmdline + KVIRTUAL_ADDRESS;
        video->print("Command line: ", reinterpret_cast<char*>(cmdline), "\n\n");
    }
    
    if (bootinfo->flags & MULTIBOOT_INFO_VBE_INFO) {
        video->print("VBE mode: ", bootinfo->vbe_mode);
        video->print(", iface segment: ", bootinfo->vbe_interface_seg);
        video->print(", iface offset: ", bootinfo->vbe_interface_off);
        video->print(", iface length: ", bootinfo->vbe_interface_len, "\n\n");
    }
   
    if (bootinfo->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) {
        video->print("Framebuffer address: ", bootinfo->framebuffer_addr);
        video->print(", width: ", bootinfo->framebuffer_width);
        video->print(", height: ", bootinfo->framebuffer_height);
        video->print(", bpp: ", bootinfo->framebuffer_bpp);
        video->print(", pitch: ", bootinfo->framebuffer_pitch, '\n');

        video->prints("Framebuffer type: ");
        switch (bootinfo->framebuffer_type) {
            case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
                video->prints("Indexed\n");
                break;
            case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
                video->prints("RGB\n");
                break;
            case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
                video->prints("EGA Text\n");
                break;
            default:
                video->prints("Unknown\n");
                break;
        }
        video->prints("\n");
    }

    video->prints("Scanning PCI devices...\n");
    auto &pci = bus::get_pci(arch);
    pci.scan_hardware();

    auto timer = peripherals::add_timer(arch, 100);
    peripherals::add_keyboard(arch);

    // Print memory information
    memory::print_memory_info();

    /*
    while (true) {
        arch->cpu_halt();
    }
    */
}