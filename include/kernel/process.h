#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#define MAX_OPEN_FILES 10
#define MAX_PATH 255
#define KERNEL_PAGE_DIRECTORY_INDEX 768

#include <kernel/pager.h>
#include <kernel/elf.h>
#include <fs/fat.h>

typedef struct task_struct{
	void *esp;
    void *esp0;
    void *cr3;
    struct task_struct *next;
    void *eip;
    int id;
    uint8_t state;
	ELF32_File *ELFfile;
    uint8_t openedFiles;
    FAT_File *fd[5];
} task_struct;

int load_process(task_struct *process, DISK *disk, const char* path);

#endif