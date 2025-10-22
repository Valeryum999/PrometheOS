#include <kernel/process.h>
#include <kernel/elf.h>

uint32_t kernel_page_directory;

task_struct load_process(void *buf){
    uint32_t *process_pd = (uint32_t *)mmap((void *)0x500000, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
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
    void *stack = mmap((void *)0x800000, PAGE_WRITABLE | PAGE_USER | PAGE_PRESENT);
    process.esp = stack + 0xfc0;
    return process;
}

void start_process(task_struct process){
    __asm__ volatile("jmp *%0"::"r"(process.ELFfile->header->ProgramEntryPosition));
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