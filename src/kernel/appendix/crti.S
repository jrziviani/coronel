// This file is part of the C runtime initialization (CRT) process. 
// It defines the `_init` and `_fini` functions, which are used by the 
// runtime linker and the compiler to handle initialization and cleanup 
// tasks for a program. These functions are typically used in conjunction 
// with other CRT files like crtbegin.o and crtend.o.

// The `.init` section is used for initialization code that runs before 
// the main function of the program. The `_init` function is called 
// automatically by the runtime loader during program startup.
.section .init
.global _init
.type _init, @function
_init:
    push    %rbp
    movq    %rsp, %rbp
    /* gcc will put contents of crtbegin.o .init section here */

// The `.fini` section is used for cleanup code that runs after the 
// main function has completed. The `_fini` function is called 
// automatically by the runtime loader during program termination.
.section .fini
.global _fini
.type _fini, @function
_fini:
    push    %rbp
    movq    %rsp, %rbp
    /* gcc will put contents of crtbegin.o .fini section here */