#include "libs/multiboot.hpp"
#include "drivers/bus/pci.hpp"

#include "archs.hpp"
#include "config.hpp"
#include "drivers/peripherals/keyboard.hpp"

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
    video->prints("Welcome to CoronelOS!\n");
    video->prints("Arch: ");
    video->prints(archs::get_arch_name());
    video->prints("\n");

    if (bootinfo->flags & MULTIBOOT_INFO_CMDLINE) {
        uintptr_t cmdline = bootinfo->cmdline + KVIRTUAL_ADDRESS;
        video->prints("Command line: ");
        video->prints(reinterpret_cast<char*>(cmdline));
        video->prints("\n\n");
    }
    
    if (bootinfo->flags & MULTIBOOT_INFO_VBE_INFO) {
        video->prints("VBE mode: ");
        video->printx(bootinfo->vbe_mode);
        video->prints(", iface segment: ");
        video->printx(bootinfo->vbe_interface_seg);
        video->prints(", iface offset: ");
        video->printx(bootinfo->vbe_interface_off);
        video->prints(", iface length: ");
        video->printx(bootinfo->vbe_interface_len);
        video->prints("\n\n");
    }
   
    if (bootinfo->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) {
        video->prints("Framebuffer address: 0x");
        video->printx(bootinfo->framebuffer_addr);
        video->prints(", width: ");
        video->printd(bootinfo->framebuffer_width);
        video->prints(", height: ");
        video->printd(bootinfo->framebuffer_height);
        video->prints(", bpp: ");
        video->printd(bootinfo->framebuffer_bpp);
        video->prints(", pitch: ");
        video->printd(bootinfo->framebuffer_pitch);
        video->prints("\n");
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

    peripherals::add_keyboard(arch);

    /*
    while (true) {
        arch->cpu_halt();
    }
    */
}