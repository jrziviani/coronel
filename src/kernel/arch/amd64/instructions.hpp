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

    void lidt(uint64_t idt);

    paddr_t get_current_page();
    void set_page_directory(paddr_t page_dir);

    void tlb_flush(paddr_t addr);
    void io_wait();
}

#endif // INSTRUCTIONS_HPP