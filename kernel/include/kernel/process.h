#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#define MAX_OPEN_FILES 10
#define MAX_PATH 255

#include <kernel/pager.h>
#include <kernel/elf.h>

typedef struct task_struct{
	void *esp;
    void *esp0;
    void *cr3;
    struct task_struct *next;
    void *eip;
    uint8_t state;
	ELF32_File *ELFfile;
} task_struct;

task_struct load_process(void *buf);
void start_process(task_struct process);

#endif