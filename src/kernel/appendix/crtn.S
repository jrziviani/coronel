// This file is part of the C runtime termination (CRT) process. 
// It complements `crti.S` and is used to finalize the initialization 
// and cleanup tasks for a program. This file typically works in 
// conjunction with other CRT files like crtbegin.o and crtend.o.

// The `.init` section here is used to finalize the initialization 
// process. It contains code that runs after the initialization code 
// from crtbegin.o. This ensures that all necessary setup is completed 
// before the program's main function is executed.
.section .init
    /* gcc will put contents of crtend.o .init section here */
    popq    %rbp
    ret

// The `.fini` section here is used to finalize the cleanup process. 
// It contains code that runs after the cleanup code from crtbegin.o. 
// This ensures that all necessary teardown tasks are completed after 
// the program's main function has finished execution.
.section .fini
    /* gcc will put contents of crtend.o .fini section here */
    popq    %rbp
    ret