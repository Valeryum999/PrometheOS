#ifndef _KERNEL_PAGER_ALLOCATOR_H
#define _KERNEL_PAGER_ALLOCATOR_H

#include <kernel/page_frame_allocator.h>

#define KERNEL_LOW_BASE 0x100000
#define KERNEL_BASE 0xc0000000
#define PAGE_DIRECTORY_SIZE 0x400000
#define PAGE_SIZE 4096  // Size of each page (4KB)
#define PAGE_PRESENT    0x1  // Page is present in memory
#define PAGE_WRITABLE   0x2  // Page is writable
#define PAGE_USER       0x4  // Page is accessible in user mode

extern uint32_t *virtual_page_directory;

void init_paging(void);
void init_page_tables();
void kalloc_page_tables(uint32_t *virtualaddr);
void *get_physaddr(void *virtualaddr);
void map_page(void *physaddr, void *virtualaddr, unsigned int flags);
void load_page_directory(uint32_t page_directory_address);
void enable_paging();

#endif