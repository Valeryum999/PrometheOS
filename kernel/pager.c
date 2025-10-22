#include <kernel/pager.h>

uint32_t page_directory[1024] __attribute__((aligned(4096), section(".lowdata")));
uint32_t first_page_table[1024] __attribute__((aligned(4096), section(".lowdata")));
// uint32_t kernel_page_table[1024] __attribute__((aligned(4096), section(".lowdata")));
//terrible workaround, to fix dynamically (maybe mmap the page tables at [endkernel-KERNEL_BASE:endkernel-KERNEL_BASE+0x400000])
// uint32_t second_kernel_page_table[1024] __attribute__((aligned(4096), section(".lowdata")));
uint32_t *virtual_page_directory = (uint32_t *) 0xfffff000;

extern uint32_t end_lowtext;
extern uint32_t end_kernel;

__attribute__((section(".lowtext"))) void init_paging(void){
    uint32_t end_kernel_addr = (uint32_t)&end_kernel;
    uint32_t start_frame = ((end_kernel_addr - KERNEL_BASE) & ~(0xfff));
    
    page_directory[0] = ((uint32_t) first_page_table | PAGE_WRITABLE | PAGE_PRESENT);
    int last_page_directory_entry_kernel = end_kernel_addr >> 22;
    //alloc and map page tables of pd[1023] themselves, 4MiB spanning after the kernel
    //TODO: fix 0
    for(int i=1; i<1023; i++){
        page_directory[i] = (start_frame + (PAGE_SIZE * i)) | PAGE_WRITABLE | PAGE_PRESENT;
    }

    //alloc kernel page tables
    for(int i=768; i<=last_page_directory_entry_kernel; i++){
        int page_directory_increment = PAGE_SIZE * (i-768); 
        uint32_t *kernel_page_table = (uint32_t *)(start_frame + i * PAGE_SIZE);
        //TODO: stop allocating page tables when all the kernel is mapped (for now it maps even beyond until the end of the pd)
        for(int j = 0; j < 1024; j++){
            kernel_page_table[j] = (KERNEL_LOW_BASE + page_directory_increment + j * PAGE_SIZE) | PAGE_WRITABLE | PAGE_PRESENT;
        }
    }
    
    //TODO: fix vga at 0x68000 + stuff needs to be mapped either manually or by page fault handler
    for(int i = 0; i < 257; i++){
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_WRITABLE | PAGE_PRESENT;
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

void malloc_page_table(uint32_t page_directory_index){
    uint32_t *page_table = (uint32_t *)malloc();
    virtual_page_directory[page_directory_index] = ((uint32_t)page_table) | PAGE_USER | PAGE_WRITABLE | PAGE_PRESENT;
}

void *mmap(void *virtualaddr, unsigned int flags) {
    //TODO: Make sure that both addresses are page-aligned.

    uint32_t page_directory_index = (uint32_t)virtualaddr >> 22;
    uint32_t page_table_index = (uint32_t)virtualaddr >> 12 & 0x3FF;

    uint32_t *page_table = (uint32_t *)(0xffc00000 + page_directory_index*PAGE_SIZE);
    void *physaddr = malloc();
    if(!(virtual_page_directory[page_directory_index] & PAGE_PRESENT)){
        malloc_page_table(page_directory_index);
    }
    //OTHERWISE swap
    page_table[page_table_index] = ((uint32_t)physaddr) | (flags & 0xFFF) | PAGE_PRESENT;


    return virtualaddr;
    // printf("page table entry at %d: %x\n",page_table_index,page_table[page_table_index]);
    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}
