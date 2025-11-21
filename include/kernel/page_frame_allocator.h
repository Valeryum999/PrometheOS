#ifndef _KERNEL_PAGE_FRAME_ALLOCATOR_H
#define _KERNEL_PAGE_FRAME_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

#define MAX_PAGES 10000
#define PAGE_SIZE 4096  // Size of each page (4KB)

typedef struct {
    uint8_t *arr[MAX_PAGES];
    int top;
} stack_t;

void init_stack();
void *kalloc_page_frame();
void *kalloc(size_t page_frames);
void free(void *ptr);
void free_page_frames(void *start_addr, size_t page_frames);

#endif