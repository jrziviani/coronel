/* This linker script is used to define the memory layout of the kernel for the 
   AMD64 architecture. It specifies how different sections of the kernel binary 
   are placed in memory, both in terms of their virtual memory addresses (VMA) 
   and their physical memory addresses (LMA). This script is critical for ensuring 
   that the kernel is loaded and executed correctly by the bootloader. */

/* OUTPUT_FORMAT specifies the binary format of the output file.
   In this case, the output format is ELF for 64-bit x86-64 architecture. */
OUTPUT_FORMAT(elf64-x86-64)

/* Define the physical load address (LMA) of the kernel.
   The bootloader will load the kernel at this address in physical memory. */
KERNEL_LMA = 0x100000;

/* Define the virtual memory address (VMA) of the kernel.
   This is the address where the kernel will be mapped in virtual memory. */
KERNEL_VMA = 0xffffffff80000000;

/* Define the entry point of the kernel.
   The bootloader will jump to this symbol to start executing the kernel. */
ENTRY(_start32)
SECTIONS
{
    /* Set the initial location counter to the virtual address where the 
       bootloader will load the kernel. This ensures that all sections 
       are placed relative to the kernel's virtual memory address. */
    . = KERNEL_VMA + KERNEL_LMA;

    /* Define the start of the kernel. */
    _start = .;

    /* Define the `.text` section, which contains executable code.
       Align it to a 4K boundary for performance and compatibility.
       The `AT` directive specifies the physical address (LMA) where 
       this section will be loaded. */
    .text ALIGN (4K) : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.multiboot)
        *(.start)
        *(.text)
        *(.text.*)
    }

    /* Define the `.rodata` section, which contains read-only data.
       Align it to a 4K boundary and specify its physical load address. */
    .rodata ALIGN (4K) : AT(ADDR(.rodata) - KERNEL_VMA)
    {
        *(.rodata)
    }

    /* Define the `.got` section, which contains the Global Offset Table (GOT).
       Align it to a 4K boundary and specify its physical load address. */
    .got ALIGN (4K) : AT(ADDR(.got) - KERNEL_VMA)
    {
        _got = .;
        KEEP(*(.got.plt))
        KEEP(*(.got))
        _egot = .;
    }

    /* Define the `.data` section, which contains read-write initialized data.
       Align it to a 4K boundary and specify its physical load address. */
    .data ALIGN (4K) : AT(ADDR(.data) - KERNEL_VMA)
    {
        *(.data)
    }

    /* Define the `.eh_frame` section, which contains exception handling information.
       Align it to a 4K boundary and specify its physical load address. */
    .eh_frame : AT(ADDR(.eh_frame) - KERNEL_VMA) {
        _eh_frame = .;
        *(.eh_frame)
        *(.eh_frame_hdr)
    }

    /* Define the end of the initialized data sections. */
    _edata = .;

    /* Define the `.bss` section, which contains read-write uninitialized data.
       Align it to a 4K boundary and specify its physical load address. */
    .bss ALIGN (4K) : AT(ADDR(.bss) - KERNEL_VMA)
    {
        _bss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /* Define the end of the kernel image. */
    _end = .;
}