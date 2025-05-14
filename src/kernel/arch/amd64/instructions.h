#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <libs/stdint.h>

namespace insn
{
    void sti();
    void cli();
    void hlt();
    void pause();
    void breakpoint();
    
    void outb(uint16_t port, uint8_t val);
    void outw(uint16_t port, uint16_t val);
    void outl(uint16_t port, uint32_t val);
    uint8_t inb(uint16_t port);
    uint16_t inw(uint16_t port);
    uint32_t inl(uint16_t port);
}

#endif // INSTRUCTIONS_H