.global i686_outb
i686_outb:
    mov 4(%esp), %dx
    mov 8(%esp), %al
    out %al, %dx
    ret

.global i686_inb
i686_inb:
    mov 4(%esp), %dx
    xor %eax, %eax
    in %dx, %al
    ret

.global i686_Panic
i686_Panic:
    cli
    hlt

.global i686_EnableInterrupts
i686_EnableInterrupts:
    sti
    ret

.global i686_DisableInterrupts
i686_DisableInterrupts:
    cli
    ret