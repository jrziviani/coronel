#ifndef IRQ_H
#define IRQ_H

#include <arch/amd64/registers.h>

void interrupt_handler(const interrupt_t &interrupt);

#endif // IRQ_H