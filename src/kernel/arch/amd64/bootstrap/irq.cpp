#include "irq.h"

#include <arch/amd64/instructions.h>
#include <arch/amd64/video/protected_mode.h>

void timer_handler(const interrupt_t &interrupt)
{
}

void keyboard_handler(const interrupt_t &interrupt)
{
    auto status = insn::inb(0x64);
    if ((status & 0x01) == 0) {
        return;
    }

    auto data = insn::inb(0x60);

    protected_mode video;
    video.printc(data);
}

void interrupt_handler(const interrupt_t &interrupt)
{
    if (interrupt.int_no == 0x28) {
        insn::outb(0xa0, 0x20);
    }

    insn::outb(0x20, 0x20);

    switch (interrupt.int_no) {
        case 0x0: // Timer interrupt
            timer_handler(interrupt);
            break;
        case 0x1: // Keyboard interrupt
            keyboard_handler(interrupt);
            break;
        case 0x3: // Serial port 1 (COM2)
            break;
        case 0x4: // Serial port 0 (COM1)
            break;
        case 0x6: // Floppy disk controller
            break;
        case 0x7: // Parallel port
            break;
        case 0x12: // PS/2 mouse
            break;
        case 0x14: // Primary IDE controller
            break;
        case 0x15: // Secondary IDE controller
            break;
        default:
            // Handle other interrupts or ignore
            break;
    }
}