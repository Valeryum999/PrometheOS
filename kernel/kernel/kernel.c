#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/pager.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/page_frame_allocator.h>
#include <fs/fat.h>

extern uint32_t end_lowtext;
extern uint32_t end_kernel;

void kernel_main(void) {
	terminal_initialize();
	init_GDT();
	init_IDT();
	init_ISR();
	init_stack();
	printf("Hello World!\n");
	DISK *disk = (DISK *) 0xc001b000; //0xc001b000 : 0xc0183000
	FAT_Initialize(disk);
}