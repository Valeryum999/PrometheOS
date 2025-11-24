#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#define MAX_OPEN_FILES 10
#define MAX_PATH 255
#define KERNEL_PAGE_DIRECTORY_INDEX 768

#include <kernel/pager.h>
#include <kernel/elf.h>
#include <fs/fat.h>

#define TASK_RUNNING            0x0000
#define TASK_INTERRUPTIBLE      0x0001
#define TASK_UNINTERRUPTIBLE    0x0002
#define __TASK_STOPPED          0x0004
#define __TASK_TRACED           0x0008
#define TASK_DEAD               0x0080
#define TASK_WAKEKILL           0x0100
#define TASK_WAKING             0x0200
#define TASK_NOLOAD             0x0400
#define TASK_NEW                0x0800

void write_cr3(uint32_t page_directory);
int load_process(task_struct *process, DISK *disk, const char* path);
int change_to_new_executable(task_struct *process, DISK *disk, const char *path);

#endif