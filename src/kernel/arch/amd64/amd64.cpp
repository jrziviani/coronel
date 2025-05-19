#include "amd64.h"
#include "instructions.h"
#include "bootstrap/segments.h"

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