#include <kernel/process.h>

uint32_t kernel_page_directory;
uint32_t count_process_id = 0;
extern void __attribute__((naked)) task_entry(void *first_eip);

void *map_page_directory_kernel(){
    uint32_t *process_pd = (uint32_t *)mmap(NULL, PAGE_SIZE, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER, MAP_ANONYMOUS, -1, 0);
    void *phys_addr = get_physaddr((void *)process_pd);
    for(int i=KERNEL_PAGE_DIRECTORY_INDEX; i<KERNEL_PAGE_DIRECTORY_INDEX+6; i++){
        process_pd[i] = virtual_page_directory[i];
    }
    //for being able to write in 0xb8000 VGA memory
    process_pd[0] = virtual_page_directory[0];
    process_pd[1023] = (uint32_t)phys_addr | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    kernel_page_directory = virtual_page_directory[1023];
    
    

    return get_physaddr((void *) process_pd);
}

void write_cr3(uint32_t pd){
    __asm__ volatile("mov %0, %%cr3" :: "r"(pd));
}

task_struct load_process_ELF(void *buf, void *page_directory){
    task_struct process;
    process.cr3 = (void *)page_directory; 
    ELF32_File file = ELF_parseFile(buf);
    process.ELFfile = &file;
    process.eip = (void *)process.ELFfile->header->ProgramEntryPosition;
    ELF_load(process.ELFfile);
    process.openedFiles = 0;
    return process;
}

void map_stack(task_struct *process){
    void *stack = mmap(NULL, 4*PAGE_SIZE, PAGE_WRITABLE | PAGE_USER | PAGE_PRESENT, MAP_ANONYMOUS, -1, 0);
    process->esp = stack + 0xfcc + 3*PAGE_SIZE;
    process->esp0 = stack + 4*PAGE_SIZE;
    uint32_t *buffer = (uint32_t *)(stack + 0xfdc + 3*PAGE_SIZE);
    *buffer = (uint32_t)task_entry;
    buffer[1] = (uint32_t)process->eip;
}

task_struct load_process(void *buf){
    void *process_pd = map_page_directory_kernel();
    write_cr3((uint32_t)process_pd);

    task_struct process = load_process_ELF(buf, process_pd);

    map_stack(&process);

    write_cr3(kernel_page_directory & ~0xfff);
    
    process.id = count_process_id++;
    return process;
}
