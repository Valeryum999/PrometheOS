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
extern void kpanic();

#define DISK_HEADER 0x6d903ceb

DISK *disk;

void timer(Registers *regs){

}

extern stack_t stack;

void kernel_main(void) {
	terminal_initialize();
	init_GDT();
	init_IDT();
	init_ISR();
	init_stack();
	void *TSS_stack = mmap((void *)0xcfff8000, 0x8000, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
	load_TSS((uint32_t)(TSS_stack + 0x8000));
	i686_IRQ_Initialize();
	i686_IRQ_RegisterHandler(0, timer);
	init_keyboard();
	printf("Hello World!\n");
	printf("End kernel is @ %x\n", &end_kernel);
	uint32_t end_kernel_uint = (uint32_t)&end_kernel;
	end_kernel_uint &= ~0xfff;
	end_kernel_uint += PAGE_SIZE;
	uint32_t *guess_disk_pos = (uint32_t*)end_kernel_uint;
	int count = 0;
	printf("Searching initrd from 0x%x\n", guess_disk_pos);
	while(*guess_disk_pos != DISK_HEADER && count < 100){
		guess_disk_pos += 0x400; // to add a pagesize
		count++;
	}
	if(count == 100){
		printf("COuldn't find disk!\n");
		kpanic();
	}

	printf("Found disk at 0x%x!\n", guess_disk_pos);
	disk = (DISK *) guess_disk_pos;
	FAT_Initialize(disk);
	printf("stack top is @ %x\n", stack.top);
	task_struct *processTestMlibc = mmap((void *)0xd0000000, 0x1000, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
	task_struct idle_task;
	initialize_multiprocessing(&idle_task);
	add_process_to_schedule(processTestMlibc);
	// add_memory_mapping(processTestMlibc, 0, PROT_READ|PROT_WRITE, 0x100000, 0, "[VGA + FAT]");
	load_process(processTestMlibc, disk, "/usr/bin/fork");
	// add_process_to_schedule(processTestMlibc2);
	schedule();
	print_memory_mappings(processTestMlibc);
	// schedule();
	// i686_IRQ_RegisterHandler(0, schedule);
	printf("stack top is @ %x\n", stack.top);
	while(1){
		
	}
}