#include "amd64.h"
#include "instructions.h"

void amd64::cpu_halt()
{
    insn::sti();
    insn::hlt();
    insn::cli();
}