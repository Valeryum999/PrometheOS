#ifndef _KERNEL_PAGE_FRAME_ALLOCATOR_H
#define _KERNEL_PAGE_FRAME_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

#define MAX_PAGES 1000  // 1043000
#define PAGE_SIZE 4096  // Size of each page (4KB)

void init_stack();
void *malloc();
void free(void *ptr);

#endif