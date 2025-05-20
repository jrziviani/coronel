#include "amd64.hpp"
#include "instructions.hpp"
#include "bootstrap/segments.hpp"

amd64::amd64()
{
    idt_setup();
    gdt_setup();
    map_kernel_memory();
}

void amd64::cpu_halt()
{
    insn::sti();
    insn::hlt();
    insn::cli();
}