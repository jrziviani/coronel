#include "amd64.h"

void amd64::cpu_halt()
{
    __asm__ __volatile__("sti");
    __asm__ __volatile__("hlt");
    __asm__ __volatile__("cli");
}