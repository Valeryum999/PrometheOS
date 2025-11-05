.macro  ISR_NOERRORCODE num
.global  ISR\num
ISR\num:
    pushl   $0
    pushl   $\num
    jmp     isr_common
.endm

.macro ISR_ERRORCODE num
.global ISR\num
ISR\num:
    pushl   $\num
    jmp     isr_common
.endm

.include "arch/i386/isrs_gen.inc"

isr_common:
    pushal
    xor %eax, %eax
    mov %ds, %ax
    push %eax       # add ds to the registers

    mov $0x10, %ax  # set all segment registers to 0x10
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp
    call ISR_Handler
    add $4, %esp

    pop %eax        #restore all segments previously stored
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popal
    add $8, %esp
    iret