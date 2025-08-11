#include "irq.hpp"

#include "arch/amd64/instructions.hpp"
#include "arch/amd64/video/protected_mode.hpp"

const timer_handler_t *timer_handler_p = nullptr;
const keyboard_handler_t *keyboard_handler_p = nullptr;

enum PIC_CONTROLLER {
    PIC_MASTER = 0x20,
    PIC_SLAVE = 0xA0
};

enum PIC_COMMAND {
    PIC_EOI = 0x20,
    PIC_INIT = 0x11,
    PIC_ICW1 = 0x01,
    PIC_ICW4 = 0x01
};

enum PIC_IRQ {
    IRQ_TIMER = 0x20,
    IRQ_KEYBOARD = 0x21,
    IRQ_SERIAL1 = 0x23,
    IRQ_SERIAL2 = 0x24,
    IRQ_FLOPPY = 0x26,
    IRQ_PARALLEL = 0x27,
    IRQ_MOUSE = 0x2C,
    IRQ_IDE1 = 0x2E,
    IRQ_IDE2 = 0x2F
};

void interrupt_handler(const interrupt_t &interrupt)
{
    // Send end-of-interrupt (EOI) signal to the slave PIC controller (if the interrupt
    // is from a slave PIC)
    if (interrupt.int_no >= 0x28) {
        insn::outb(PIC_CONTROLLER::PIC_SLAVE, PIC_COMMAND::PIC_EOI);
    }

    // Send end-of-interrupt (EOI) signal to the master PIC controller
    insn::outb(PIC_CONTROLLER::PIC_MASTER, PIC_COMMAND::PIC_EOI);

    switch (interrupt.int_no) {
        case PIC_IRQ::IRQ_TIMER:
            if (timer_handler_p != nullptr) {
                (*timer_handler_p)(interrupt);
            }
            break;

        case PIC_IRQ::IRQ_KEYBOARD:
            if (keyboard_handler_p != nullptr) {
                (*keyboard_handler_p)(interrupt);
            }
            break;

        case PIC_IRQ::IRQ_SERIAL1:
            break;

        case PIC_IRQ::IRQ_SERIAL2:
            break;

        case PIC_IRQ::IRQ_FLOPPY:
            break;

        case PIC_IRQ::IRQ_PARALLEL:
            break;

        case PIC_IRQ::IRQ_MOUSE:
            break;

        case PIC_IRQ::IRQ_IDE1:
            break;

        case PIC_IRQ::IRQ_IDE2:
            break;

        default:
            break;
    }
}