#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/pager.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/irq.h>
#include <kernel/io.h>
#include <kernel/keyboard.h>
#include <kernel/cursor.h>
#include <kernel/page_frame_allocator.h>
#include <kernel/elf.h>
#include <kernel/process.h>
#include <kernel/scheduler.h>
#include <fs/fat.h>

extern uint32_t end_lowtext;
extern uint32_t end_kernel;

void timer(Registers *regs){
	
}

void kernel_main(void) {
	terminal_initialize();
	init_GDT();
	init_IDT();
	init_ISR();
	i686_IRQ_Initialize();
	i686_IRQ_RegisterHandler(0, timer);
	init_stack();
	init_keyboard();
	printf("Hello World!\n");
	uint32_t disk = 0xc0024000;
	uint32_t helloA = disk + 0x7a00;
	uint32_t helloB = disk + 0x9c00;
	uint32_t helloC = disk + 0xbe00;
	task_struct processA = load_process((void *)helloA);
	task_struct processB = load_process((void *)helloB);
	task_struct processC = load_process((void *)helloC);
	add_process_to_schedule(&processA);
	add_process_to_schedule(&processB);
	add_process_to_schedule(&processC);
	i686_IRQ_RegisterHandler(0, schedule);
	while(1){
		
	}
}