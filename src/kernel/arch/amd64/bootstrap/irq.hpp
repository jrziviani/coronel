#ifndef IRQ_HPP
#define IRQ_HPP

#include "drivers/peripherals/keyboard.hpp"
#include "arch/amd64/registers.hpp"

extern const keyboard_handler_t *keyboard_handler_p;

void interrupt_handler(const interrupt_t &interrupt);

#endif // IRQ_HPP