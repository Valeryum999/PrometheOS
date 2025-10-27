#include <kernel/process.h>
#include <kernel/elf.h>

uint32_t kernel_page_directory;
extern void __attribute__((naked)) task_entry(void *first_eip);

task_struct load_process(void *buf){
    uint32_t *process_pd = (uint32_t *)mmap(NULL, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    for(int i=768; i<774; i++){
        printf("Virtual page directory @ %d: %x\n", i, virtual_page_directory[i]);
        process_pd[i] = virtual_page_directory[i];
    }
    process_pd[0] = virtual_page_directory[0];
    process_pd[1023] = (uint32_t)process_pd | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    kernel_page_directory = virtual_page_directory[1023];
    printf("pd should point to itself: %x\n", process_pd[1023]);
    task_struct process;
    process.cr3 = (void *)process_pd;
    __asm__ volatile("mov %0, %%cr3" :: "r"(process.cr3));
    ELF32_File file = ELF_parseFile(buf);
    process.ELFfile = &file;
    process.eip = (void *)process.ELFfile->header->ProgramEntryPosition;
    printf("process eip!! %x %x\n", process.eip, process.ELFfile->header->ProgramEntryPosition);
    ELF_load(process.ELFfile);
    void *stack = mmap(NULL, PAGE_WRITABLE | PAGE_USER | PAGE_PRESENT);
    process.esp = stack + 0xfcc;
    process.esp0 = stack + 0x1000;
    uint32_t *buffer = (uint32_t *)(stack + 0xfdc);
    *buffer = (uint32_t)task_entry;
    *(buffer+1) = (uint32_t)process.eip;
    printf("Stack is mapped at %x\n", process.esp0 - 0x1000);
    __asm__ volatile("mov %0, %%cr3" :: "r"(kernel_page_directory & ~0xfff));
    return process;
}
