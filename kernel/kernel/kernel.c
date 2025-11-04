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

DISK *disk = (DISK *)0xc0028000;

void timer(Registers *regs){
	
}

// 0xc002a400 //hi there
// 0xc002c600	
// 0xc002c800 //halt
// 0xc002fc00 //b
// 0xc0031e00 //c

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
	FAT_Initialize(disk);
	FAT_printBootSector();
	uint32_t helloA = (uint32_t)(disk) + 0x4400;
	// uint8_t fileBuf[0x2230];
	// printf("after initialize\n");
	// FAT_File *file = FAT_Open(disk, "/");
	// printf("after fat open\n");
	// FAT_DirectoryEntry *dirEntry;
	// int count = 0;
	// while(FAT_ReadEntry(disk, file, dirEntry) && count < 20){
	// 	printf("	%s\n", dirEntry->Name);
	// 	count++;
	// }
	// FAT_Read(disk, file, 0x2230, fileBuf);
	// printf("after fat read\n");
	// uint32_t helloB = (uint32_t)(disk) + 0x9c00;
	// uint32_t helloC = (uint32_t)(disk) + 0xbe00;
	task_struct processA = load_process((void *)helloA);
	printf("after load process\n");
	// task_struct processB = load_process((void *)helloB);
	// task_struct processC = load_process((void *)helloC);
	task_struct idle_task;
	initialize_multiprocessing(&idle_task);
	add_process_to_schedule(&processA);
	// add_process_to_schedule(&processB);
	// add_process_to_schedule(&processC);
	i686_IRQ_RegisterHandler(0, schedule);
	while(1){
		
	}
	kpanic();
}