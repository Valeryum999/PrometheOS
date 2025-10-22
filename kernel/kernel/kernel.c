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
	// FAT_Initialize(disk);
	uint32_t disk = 0xc0024000;
	uint32_t elf = disk + 0x4400;
	uint32_t halt = disk + 0x6800;
	printf("ELF starts @ %x\n",elf);
	task_struct process = load_process((void *)elf);
	// start_process(process);
	add_process_to_schedule(&process);
	i686_IRQ_RegisterHandler(0, schedule);
	while(1){

	}
	// schedule();
}