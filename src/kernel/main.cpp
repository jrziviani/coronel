#include <libs/multiboot.h>
#include "archs.h"

void kmain(multiboot_info_t *bootinfo, unsigned long magic)
{
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        return;
    }

    if (bootinfo->flags & MULTIBOOT_INFO_VBE_INFO) {
        // VBE information is available
        // You can access the VBE information here
    }

    if (bootinfo->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) {
        // Framebuffer information is available
        // You can access the framebuffer information here
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

    while (true) {
        arch->cpu_halt();
    }
}