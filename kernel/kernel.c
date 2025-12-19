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
#include <kernel/logging.h>

extern uint32_t end_lowtext;
extern uint32_t end_kernel;
extern void kpanic();
extern void reload_GDT_for_TSS();

#define DISK_HEADER 0x6d903ceb

DISK *disk;
void *end_of_disk;

void timer(Registers *regs){

}

extern stack_t stack;

void kernel_main(uint32_t mb2_magic, uint32_t mb_info_addr) {
	flanterm_initialize(mb2_magic, mb_info_addr);
	init_GDT();
	init_IDT();
	init_ISR();
	init_stack();
	void *TSS_stack = mmap((void *)0xcfff8000, 0x8000, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
	load_TSS((uint32_t)(TSS_stack + 0x8000));
	reload_GDT_for_TSS();
	i686_IRQ_Initialize();
	i686_IRQ_RegisterHandler(0, timer);
	init_keyboard();
	// printf("End kernel is @ 0x%x phys_addr: 0x%x\n", &end_kernel, get_physaddr((void *)&end_kernel));
	uint32_t end_kernel_uint = (uint32_t)&end_kernel;
	end_kernel_uint &= ~0xfff;
	end_kernel_uint += PAGE_SIZE;
	uint32_t *guess_disk_pos = (uint32_t*)end_kernel_uint;
	int count = 0;
	// printf("Searching initrd from 0x%x\n", guess_disk_pos);
	while(*guess_disk_pos != DISK_HEADER && count < 100){
		guess_disk_pos += 0x400; // to add a pagesize
		count++;
	}
	if(count == 100){
		error_print("Couldn't find disk!\n");
		kpanic();
	}

	ok_print("Found initrd");
	disk = (DISK *) guess_disk_pos;
	end_of_disk = get_physaddr((void *)(guess_disk_pos + 0x1FFF000));
	FAT_Initialize(disk);
	task_struct *processTestMlibc = mmap((void *)0xd0000000, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
	task_struct idle_task;
	initialize_multiprocessing(&idle_task);
	add_process_to_schedule(processTestMlibc);
	load_process(processTestMlibc, disk, "/usr/bin/bash", NULL, NULL);
	while(1){
		schedule();
	}
}