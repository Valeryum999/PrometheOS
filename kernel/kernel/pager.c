#include <kernel/pager.h>
#include <stdio.h>

uint32_t page_directory[1024] __attribute__((aligned(4096), section(".lowdata")));
uint32_t first_page_table[1024] __attribute__((aligned(4096), section(".lowdata")));
uint32_t kernel_page_table[1024] __attribute__((aligned(4096), section(".lowdata")));
uint32_t second_kernel_page_table[1024] __attribute__((aligned(4096), section(".lowdata")));
uint32_t *virtual_page_directory = (uint32_t *) 0xfffff000;

__attribute__((section(".lowtext"))) void init_paging(void){
    for(int i = 0; i < 1024; i++){
        page_directory[i] = PAGE_WRITABLE;
    }
    
    page_directory[0] = ((uint32_t) first_page_table | PAGE_WRITABLE | PAGE_PRESENT);
    page_directory[768] = ((uint32_t) kernel_page_table | PAGE_WRITABLE | PAGE_PRESENT);
    page_directory[769] = ((uint32_t) second_kernel_page_table | PAGE_WRITABLE | PAGE_PRESENT);
    
    for(int i = 0; i < 1024; i++){
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_WRITABLE | PAGE_PRESENT;
    }
    for(int i = 0; i < 1024; i++){
        kernel_page_table[i] = (0x100000 + i * PAGE_SIZE) | PAGE_WRITABLE | PAGE_PRESENT;
    }
    for(int i = 0; i < 1024; i++){
        second_kernel_page_table[i] = (0x500000 + i * PAGE_SIZE) | PAGE_WRITABLE | PAGE_PRESENT;
    }
    
    //setup recursive mapping
    page_directory[1023] = (uint32_t) page_directory | PAGE_WRITABLE | PAGE_PRESENT;

    //load pd into cr3 and enter paging mode by setting pg in cr0
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

void kalloc_page_tables(uint32_t *virtualaddr){
    uint32_t *page_table_entry = kalloc_frame();
    uint32_t page_directory_index = (uint32_t)virtualaddr >> 22;
    for(int i = 0; i < 1024; i++){
        page_table_entry[i] = ((uint32_t)page_table_entry + i * 0x1000) | PAGE_WRITABLE | PAGE_PRESENT;
    }
    virtual_page_directory[page_directory_index] = ((uint32_t)page_table_entry) | PAGE_WRITABLE | PAGE_PRESENT;
}

void *get_physaddr(void *virtualaddr) {
    uint32_t page_directory_index = (uint32_t)virtualaddr >> 22;
    uint32_t page_table_index = (uint32_t)virtualaddr >> 12 & 0x3FF;
    if(!(virtual_page_directory[page_directory_index] & 0x1))
        return NULL;

    uint32_t *page_table = (uint32_t *)((uint32_t)0xFFC00000 + page_directory_index * PAGE_SIZE);
    if(!(page_table[page_table_index] & 0x1))
        return NULL;

    return (void *)((page_table[page_table_index] & ~0xFFF) + ((uint32_t)virtualaddr & 0xFFF));
}

void map_page(void *physaddr, void *virtualaddr, unsigned int flags) {
    // Make sure that both addresses are page-aligned.

    uint32_t page_directory_index = (uint32_t)virtualaddr >> 22;
    uint32_t page_table_index = (uint32_t)virtualaddr >> 12 & 0x3FF;

    // unsigned long *pd = (unsigned long *)0xFFFFF000;
    if(!(virtual_page_directory[page_directory_index] & 0x1))
        kalloc_page_tables(virtualaddr);

    uint32_t *page_table = (uint32_t *) 0xffc00000 + page_directory_index*PAGE_SIZE;
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?
    // if(!(page_table[page_table_index] & 0x1))
    //     return; //error?

    page_table[page_table_index] = ((uint32_t)physaddr) | (flags & 0xFFF) | 0x01; // Present

    printf("page table entry at %d: %x\n",page_table_index,page_table[page_table_index]);
    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}
