.section .text
.global load_IDT
load_IDT:
    push %ebp
    mov %esp, %ebp

    mov 8(%esp), %eax
    lidt (%eax)

    mov %ebp, %esp
    pop %ebp
    ret