#include "irq.h"

#include <arch/amd64/instructions.h>
#include <arch/amd64/video/protected_mode.h>

const keyboard_handler_t *keyboard_handler_p = nullptr;

void interrupt_handler(const interrupt_t &interrupt)
{
    if (interrupt.int_no == 0x28) {
        insn::outb(0xa0, 0x20);
    }

    insn::outb(0x20, 0x20);

    switch (interrupt.int_no) {
        case 0x0: // Timer interrupt
            break;
        case 0x1: // Keyboard interrupt
            if (keyboard_handler_p != nullptr) {
                (*keyboard_handler_p)(interrupt);
            }
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