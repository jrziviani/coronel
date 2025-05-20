#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "libs/stdint.hpp"

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

    paddr_t get_current_page();
    void tlb_flush(paddr_t addr);

    void lidt(uint64_t idt);
}

#endif // INSTRUCTIONS_HPP