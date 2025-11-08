.global syscall_entry
syscall_entry:
    jmp executeSyscall

.global executeSyscall
executeSyscall:
    pushal
    xor %eax, %eax

    mov %ds, %ax
    push %eax       # add ds to the registers

    mov $0x10, %ax  # set all segment registers to 0x10
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    #mov %ax, %gs

    push %esp
    
    call SyscallHandler

    add $4, %esp
    mov %eax, 32(%esp)  # set return code

    pop %eax        #restore all segments previously stored
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs

    #pop %eax        #restore gs segment
    #mov $0x18, %gs

    popal
    iret