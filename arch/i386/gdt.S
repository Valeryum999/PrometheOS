.section .text
.global load_GDT
load_GDT:
    push %ebp
    mov %esp, %ebp

    mov 8(%esp), %eax
    lgdt (%eax)
    
    ljmp $0x08, $.reload_CS
.reload_CS:
    mov 16(%esp), %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    mov %ebp, %esp
    pop %ebp
    ret

.global reload_GDT
reload_GDT:
    push %ebp
    mov %esp, %ebp

    mov 8(%esp), %eax
    
    mov 12(%esp), %ax
    ljmp $0x08, $.reload_CS_GS
.reload_CS_GS:
    mov 20(%esp), %ax
    mov %ax, %gs

    mov %ebp, %esp
    pop %ebp
    ret

.global reload_GS
reload_GS:
    push %ebp
    mov %esp, %ebp

    mov 8(%esp), %eax
    mov %ax, %gs

    mov %ebp, %esp
    pop %ebp
    ret

.global reload_GDT_for_TSS
reload_GDT_for_TSS:
    push %ebp
    mov %esp, %ebp

    mov $0x2b, %ax
    ltr %ax

    mov %ebp, %esp
    pop %ebp
    ret