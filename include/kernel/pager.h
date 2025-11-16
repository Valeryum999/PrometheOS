#ifndef _KERNEL_PAGER_ALLOCATOR_H
#define _KERNEL_PAGER_ALLOCATOR_H

#include <kernel/page_frame_allocator.h>
#include <stdio.h>

#define KERNEL_LOW_BASE 0x100000
#define KERNEL_BASE 0xc0000000
#define PAGE_DIRECTORY_SIZE 0x400000
#define PAGE_SIZE 4096  // Size of each page (4KB)
#define PAGE_PRESENT    0x01  // Page is present in memory
#define PAGE_WRITABLE   0x02  // Page is writable
#define PAGE_USER       0x04  // Page is accessible in user mode

#define MAP_ANONYMOUS   0x20

#define PROT_READ       0x01
#define PROT_WRITE      0x02
#define PROT_EXECUTE    0x04

extern uint32_t *virtual_page_directory;

void init_paging(void);
void init_page_tables();
void kalloc_page_tables(uint32_t *virtualaddr);
void *get_physaddr(void *virtualaddr);
void *mmap(void *virtualaddr, size_t size, int prot, int flags, int fd, uint32_t offset);
int mprotect(void *start, size_t size, uint32_t prot);
void load_page_directory(uint32_t page_directory_address);
void enable_paging();

#endif