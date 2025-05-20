#ifndef AMD64_HPP
#define AMD64_HPP

#include "arch/iarch.hpp"
#include "bootstrap/irq.hpp"
#include "instructions.hpp"
#include "memory/paging.hpp"
#include "memory/pagetable.hpp"
#include "video/protected_mode.hpp"

class amd64 : public iarch
{
private:
    protected_mode video_;
    paging paging_;

public:
    amd64();

    void cpu_halt() override;

    iprotected_mode *get_video() override
    {
        return &video_;
    }

    void set_keyboard_handler(const keyboard_handler_t *handler) override
    {
        keyboard_handler_p = handler;
    }

    uint8_t read_byte(uint32_t addr, uint16_t addr_port, uint16_t value_port) const override
    {
        insn::outl(addr_port, addr);
        return insn::inb(value_port);
    }

    uint16_t read_word(uint32_t addr, uint16_t addr_port, uint16_t value_port) const override
    {
        insn::outl(addr_port, addr);
        return insn::inw(value_port);
    }

    uint32_t read_dword(uint32_t addr, uint16_t addr_port, uint16_t value_port) const override
    {
        insn::outl(addr_port, addr);
        return insn::inl(value_port);
    }

    void write_byte(uint32_t addr, uint16_t addr_port, uint16_t value_port, uint8_t value) const override
    {
        insn::outl(addr_port, addr);
        insn::outb(value_port, value);
    }

    void write_word(uint32_t addr, uint16_t addr_port, uint16_t value_port, uint16_t value) const override
    {
        insn::outl(addr_port, addr);
        insn::outw(value_port, value);
    }

    void write_dword(uint32_t addr, uint16_t addr_port, uint16_t value_port, uint32_t value) const override
    {
        insn::outl(addr_port, addr);
        insn::outl(value_port, value);
    }

#if 0
    uint8_t read_byte(uint32_t address) const
    {
        uint8_t value;
        asm volatile("movb %1, %0" : "=r"(value) : "m"(*(volatile uint8_t *)address));
        return value;
    }

    uint16_t read_word(uint32_t address) const
    {
        uint16_t value;
        asm volatile("movw %1, %0" : "=r"(value) : "m"(*(volatile uint16_t *)address));
        return value;
    }

    uint32_t read_dword(uint32_t address) const
    {
        uint32_t value;
        asm volatile("movl %1, %0" : "=r"(value) : "m"(*(volatile uint32_t *)address));
        return value;
    }

    void write_byte(uint32_t address, uint8_t value) const
    {
        asm volatile("movb %0, %1" : "=m"(*(volatile uint8_t *)address) : "r"(value));
    }

    void write_word(uint32_t address, uint16_t value) const
    {
        asm volatile("movw %0, %1" : "=m"(*(volatile uint16_t *)address) : "r"(value));
    }

    void write_dword(uint32_t address, uint32_t value) const
    {
        asm volatile("movl %0, %1" : "=m"(*(volatile uint32_t *)address) : "r"(value));
    }
#endif
};

#endif // AMD64_HPP