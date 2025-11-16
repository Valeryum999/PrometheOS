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

DISK *disk = (DISK *)0xc050c000;

void timer(Registers *regs){
	
}

void kernel_main(void) {
	init_GDT();
	init_IDT();
	init_ISR();
	i686_IRQ_Initialize();
	i686_IRQ_RegisterHandler(0, timer);
	init_stack();
	terminal_initialize();
	init_keyboard();
	printf("Hello World!\n");
	printf("End kernel is @ %x\n", &end_kernel);
	printf("ELF test is @ 0xc0431000\n");
	printf("physaddr disk: %x ... %x\n", get_physaddr((void *)disk), get_physaddr((void *)disk + 0x3bb9a0));
	FAT_Initialize(disk);
	FAT_printBootSector();
	//everything mapped at the same addr wtf
	// printf("%x\n", get_physaddr((void *)0xc0032000));
	// printf("%x\n", get_physaddr((void *)0xc0431000));
	// printf("%x\n", get_physaddr((void *)0xc0830000));
	// printf("%x\n", get_physaddr((void *)0xc0c2f000));
	// printf("%x\n", get_physaddr((void *)0xc102e000));
	// printf("%x\n", get_physaddr((void *)0xc142d000));
	// printf("%x\n", get_physaddr((void *)0xc182c000));
	// printf("%x\n", get_physaddr((void *)0xc1c2b000));
	// FAT_DirectoryEntry *entryOut;
	// FAT_File *test_file = FAT_Open(disk, "/usr/lib/ld.so");
	// uint8_t *buf = mmap((void *)0xd0000000, 0x1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
	// FAT_Read(disk, test_file, 0x1000, buf);
	// printf("Size: %x", test_file->Size);
	// for(int i=0; i<0x1000; i++){
	// 	printf("%x",buf[i]);
	// }
	uint32_t test = 0xc0432000;
	task_struct *processTestMlibc = mmap((void *)0xd0000000, 0x1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
	load_process(processTestMlibc, (void *)test);
	task_struct idle_task;
	initialize_multiprocessing(&idle_task);
	add_process_to_schedule(processTestMlibc);
	schedule();
	// i686_IRQ_RegisterHandler(0, schedule);
	while(1){
		
	}
	// kpanic();
}