# Syscall interrupt handler
.globl syscall_int
.type syscall_int, @function

syscall_int:
    cli
    
    # Save all registers
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
    
    # Save segment registers
    mov %ds, %ax
    push %rax
    
    # Set kernel data segment
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    
    # Syscall arguments are in: rax (syscall_no), rbx (arg1), rcx (arg2), rdx (arg3)
    # But we need to pass them as C function arguments: rdi, rsi, rdx, rcx
    # CRITICAL: Save %rdx first before it gets overwritten!
    mov %rdx, %r8   # Save original %rdx (arg3) in temp register
    mov %rax, %rdi  # syscall_no
    mov %rbx, %rsi  # arg1
    mov %rcx, %rdx  # arg2
    mov %r8, %rcx   # arg3 (from saved %rdx)
    
    # call syscall_handler
    
    # Return value is in %rax, keep it there
    
    # Restore segment registers
    pop %rbx
    mov %bx, %ds
    mov %bx, %es
    
    # Restore all registers except %rax (return value)
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    add $8, %rsp    # Skip saved %rax to keep return value
    
    sti
    iretq
