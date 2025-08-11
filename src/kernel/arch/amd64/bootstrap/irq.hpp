#ifndef IRQ_HPP
#define IRQ_HPP

#include "drivers/peripherals/timer.hpp"
#include "drivers/peripherals/keyboard.hpp"
#include "arch/amd64/registers.hpp"

extern const timer_handler_t *timer_handler_p;
extern const keyboard_handler_t *keyboard_handler_p;

void interrupt_handler(const interrupt_t &interrupt);

#endif // IRQ_HPP