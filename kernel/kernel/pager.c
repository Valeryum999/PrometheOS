#include <kernel/pager.h>
#include <stdio.h>

uint32_t page_directory[1024] __attribute__((aligned(4096), section(".lowdata")));
uint32_t first_page_table[1024] __attribute__((aligned(4096), section(".lowdata")));
uint32_t new_page_table[1024] __attribute__((aligned(4096), section(".lowdata")));
uint32_t *page_table;

__attribute__((section(".lowtext"))) void init_paging(void){
    for(int i = 0; i < 1024; i++){
        page_directory[i] = PAGE_WRITABLE;
    }
    
    page_directory[0] = ((uint32_t) first_page_table | PAGE_WRITABLE | PAGE_PRESENT);
    page_directory[768] = ((uint32_t) new_page_table | PAGE_WRITABLE | PAGE_PRESENT);
    
    for(int i = 0; i < 1024; i++){
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_WRITABLE | PAGE_PRESENT;
    }
    for(int i = 0; i < 1024; i++){
        new_page_table[i] = (0x100000 + i * 0x1000) | PAGE_WRITABLE | PAGE_PRESENT;
    }
    
    load_page_directory((uint32_t)page_directory);
    enable_paging();
}

__attribute__((section(".lowtext"))) void load_page_directory(uint32_t page_directory_address) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(page_directory_address));
}

__attribute__((section(".lowtext"))) void enable_paging() {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  // Set the paging (PG) bit
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
}

void init_recursive_page_tables(void){
    uint32_t result = (uint32_t)page_directory - 0x100000 + KERNEL_BASE;
    printf("result: %x\n", result);
    uint32_t *page_directory_virtual = (uint32_t *) result;
    printf("kekw at %x should have pte 0: %x\n",result,page_directory_virtual[0]);
    page_directory[1023] = (uint32_t) page_directory | PAGE_WRITABLE | PAGE_PRESENT;
    // uint32_t *test = (uint32_t *)0xfffff000;
    // printf("test hope: %x",test[0]);
}

void kalloc_page_table(uint32_t *virtualaddr){
    page_table = kalloc_frame();
    uint32_t page_directory_index = (uint32_t)virtualaddr >> 22;
    for(int i = 0; i < 1024; i++){
        page_table[i] = (i * 0x1000) | 3;
    }
    
    page_directory[page_directory_index] = ((uint32_t) page_table) | 3;
    printf("page directory entry at %d: %x\n",page_directory_index,page_directory[page_directory_index]);
}

void *get_physaddr(void *virtualaddr) {
    uint32_t page_directory_index = (uint32_t)virtualaddr >> 22;
    uint32_t page_table_index = (uint32_t)virtualaddr >> 12 & 0x3FF;

    // uint32_t *pd = (uint32_t *)0xFFFFF000;
    if(!(page_directory[page_directory_index] & 0x1))
        return NULL;
    // Here you need to check whether the PD entry is present.

    page_table = (uint32_t *) page_directory[page_directory_index];
    // uint32_t *pt = ((uint32_t *)0xFFC00000) + (0x400 * pdindex);
    if(!(page_table[page_table_index] & 0x1))
        return NULL;
    // Here you need to check whether the PT entry is present.

    return (void *)((page_table[page_table_index] & ~0xFFF) + ((uint32_t)virtualaddr & 0xFFF));
}

void map_page(void *physaddr, void *virtualaddr, unsigned int flags) {
    // Make sure that both addresses are page-aligned.

    uint32_t page_directory_index = (uint32_t)virtualaddr >> 22;
    uint32_t page_table_index = (uint32_t)virtualaddr >> 12 & 0x3FF;

    // unsigned long *pd = (unsigned long *)0xFFFFF000;
    if(!(page_directory[page_directory_index] & 0x1))
        kalloc_page_table(virtualaddr);

    page_table = (uint32_t *) page_directory[page_directory_index];
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?
    // if(!(page_table[page_table_index] & 0x1))
    //     return; //error?

    page_table[page_table_index] = ((uint32_t)physaddr) | (flags & 0xFFF) | 0x01; // Present

    printf("page table entry at %d: %x\n",page_table_index,page_table[page_table_index]);
    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}
