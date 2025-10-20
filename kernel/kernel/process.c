#include <kernel/process.h>
#include <kernel/elf.h>

uint32_t kernel_page_directory;

void load_process(void *buf){
    uint32_t *process_pd = (uint32_t *)mmap((void *)0x500000, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    printf("New kernel page directory: %x\n",process_pd);
    printf("Virtual page directory @ %x\n", virtual_page_directory);
    for(int i=768; i<774; i++){
        printf("Virtual page directory @ %d: %x\n", i, virtual_page_directory[i]);
        process_pd[i] = virtual_page_directory[i];
    }
    process_pd[0] = virtual_page_directory[0];
    process_pd[1023] = (uint32_t)process_pd | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    printf("pd should point to itself: %x\n", process_pd[1023]);
    Process process;
    process.page_directory = (uint32_t)process_pd;
    process.ELFfile = buf;
    start_process(process);
}

void start_process(Process process){
    kernel_page_directory = virtual_page_directory[1023];
    __asm__ volatile("mov %0, %%cr3" :: "r"(process.page_directory));
    ELF32_File file = ELF_parseFile(process.ELFfile);
    ELF_load(&file);
    __asm__ volatile("jmp *%0"::"r"(file.header->ProgramEntryPosition));
}

void exit_process(){
    printf("Kernel page directory: %x\n",kernel_page_directory & ~0xfff);
    __asm__ volatile("mov %0, %%cr3" :: "r"(kernel_page_directory & ~0xfff));
    __asm__ volatile("mov %ebp, %esp");
    __asm__ volatile("pop %ebp");
    __asm__ volatile("mov %ebp, %esp");
    __asm__ volatile("pop %ebp");
    __asm__ volatile("mov %ebp, %esp");
    __asm__ volatile("pop %ebp");
    __asm__ volatile("mov %ebp, %esp");
    __asm__ volatile("pop %ebp");
    __asm__ volatile("mov %ebp, %esp");
    __asm__ volatile("pop %ebp");
    __asm__ volatile("ret");
}