#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#define MAX_OPEN_FILES 10
#define MAX_PATH 255
#define KERNEL_PAGE_DIRECTORY_INDEX 768

#include <kernel/pager.h>
#include <kernel/elf.h>
#include <fs/fat.h>

int load_process(task_struct *process, DISK *disk, const char* path);
int change_to_new_executable(task_struct *process, DISK *disk, const char *path);

#endif