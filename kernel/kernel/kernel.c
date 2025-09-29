#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/pager.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/buddy_allocator.h>

extern uint32_t end_lowtext;
extern uint32_t end_kernel;

void kernel_main(void) {
	terminal_initialize();
	init_GDT();
	init_IDT();
	init_ISR();
	printf("Hello World!\n");
	map_page((void *)0xdeadb000,(void *)0xbeefd000,0);
	printf("dereferencing %x: %x",(void *)0xdeadb000,*(uint32_t *)0xbeefd000);
	printf("phys addr of 0xbeefd000 %x", get_physaddr((void *) 0xbeefd000));
}