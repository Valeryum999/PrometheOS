#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/pager.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/page_frame_allocator.h>
#include <kernel/elf.h>
#include <kernel/process.h>
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
	// FAT_Initialize(disk);
	uint32_t elf = 0xc0020000 + 0x4400;
	printf("ELF starts @ %x\n",elf);
	load_process((void *)elf);
	printf("After process execution! :))");
	// ELF32_File file = ELF_parseFile((void *)elf);
	// ELF_load(&file);
	// FAT_File *file = FAT_Open(disk, "hello");
	// asm volatile("jmp *%0"::"r"(file.header->ProgramEntryPosition));
}