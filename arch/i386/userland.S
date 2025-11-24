.global test_usermode
test_usermode:
    cli
    ret

.global jump_usermode
jump_usermode:
    mov $0x23, %ax #userland data segment
	mov %ax, %ds 
	mov %ax, %es  
	mov %ax, %fs  
	mov %ax, %gs

    pop %ebx #function
	mov %esp, %eax
	push $0x23 #data segment
	push %eax
	pushf
	push $0x1b #code segment
	push %ebx
    xor %ebx, %ebx
	iret