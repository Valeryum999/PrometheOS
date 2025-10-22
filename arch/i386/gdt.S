.section .text
.global load_GDT
load_GDT:
    push %ebp
    mov %esp, %ebp

    mov 8(%esp), %eax
    lgdt (%eax)
    
    mov 12(%esp), %ax
    mov %ax, %cs

    mov 16(%esp), %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    mov %ebp, %esp
    pop %ebp
    ret