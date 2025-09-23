#ifndef _KERNEL_PAGE_FRAME_ALLOCATOR_H
#define _KERNEL_PAGE_FRAME_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

enum PageStatus{
    FREE,
    USED
};

#define MAX_PAGES 1048319

extern uint8_t frame_map[MAX_PAGES];
extern uint32_t physical_frames[MAX_PAGES];
extern void *start_frame;

void init_start_frame();
void *kalloc_frame_int();
void *kalloc_frame();
void kfree_frame(uint32_t *a);

#endif