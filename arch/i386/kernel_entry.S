.section .lowentry
.extern init_paging

.set KERNEL_LOW_BASE, 0x00100000
.set KERNEL_BASE, 0xc0000000
.set KERNEL_OFFSET_DIFF, (KERNEL_HIGH_BASE-KERNEL_LOW_BASE)

.global _entry
.type _entry, @function
_entry:
    # Init paging first while running in lower half addreses
    push %eax
    push %ebx
    call init_paging

    pop %ebx
    pop %eax
    # Init stack to higher half addresses
    mov $stack_top, %esp

    push %ebx
    push %eax
    # _start label is a higher half address. After this point
    # we are running in the higher half of memory.
    call kernel_main

    hltloop:
        hlt
        jmp hltloop

# 8k stack aligned to 16 bytes in .bss section
.local stack_top, stack_bottom
.comm stack_bottom, 8192, 16
.comm stack_top, 0

.global kpanic
kpanic:
    cli
    hlt