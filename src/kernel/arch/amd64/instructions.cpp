#include "instructions.h"

void insn::sti() {
    __asm__ __volatile__("sti");
}

void insn::cli() {
    __asm__ __volatile__("cli");
}

void insn::hlt() {
    __asm__ __volatile__("hlt");
}

void insn::pause() {
    __asm__ __volatile__("pause");
}

void insn::breakpoint() {
    __asm__ __volatile__("int3");
}

void insn::outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %1, %0"
                 :
                 : "dN"(port), "a"(val));
}

void insn::outw(uint16_t port, uint16_t val)
{
    asm volatile("outw %1, %0"
                 :
                 : "dN"(port), "a"(val));
}

void insn::outl(uint16_t port, uint32_t val)
{
    asm volatile("outl %1, %0"
                 :
                 : "dN"(port), "a"(val));
}

uint8_t insn::inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0"
                 : "=a"(ret)
                 : "dN"(port));

    return ret;
}

uint16_t insn::inw(uint16_t port)
{
    uint16_t ret;
    asm volatile("inw %1, %0"
                 : "=a"(ret)
                 : "dN"(port));

    return ret;
}

uint32_t insn::inl(uint16_t port)
{
    uint32_t ret;
    asm volatile("inl %1, %0"
                 : "=a"(ret)
                 : "dN"(port));

    return ret;
}