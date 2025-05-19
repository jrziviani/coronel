#ifndef IRQ_H
#define IRQ_H

#include "drivers/peripherals/keyboard.h"
#include "arch/amd64/registers.h"

extern const keyboard_handler_t *keyboard_handler_p;

void interrupt_handler(const interrupt_t &interrupt);

#endif // IRQ_H