#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#define MAX_OPEN_FILES 10
#define MAX_PATH 255

#include <kernel/pager.h>
#include <kernel/elf.h>

typedef struct {
	int pid;
	// OpenFile open_files[MAX_OPEN_FILES];
	uint32_t page_directory;
	void *ELFfile;
} Process;

// typedef struct {
// 	int fd;
// 	char path[MAX_PATH];
// } OpenFile;

void load_process(void *buf);
void start_process(Process process);

#endif