#include <kernel/process.h>

uint32_t count_process_id = 0;
uint32_t kernel_page_directory;
ELF32_File *dynamicLoader;

extern void __attribute__((naked)) task_entry();

void write_cr3(uint32_t pd){
    __asm__ volatile("mov %0, %%cr3" :: "r"(pd));
}

void *map_page_directory_kernel(){
    uint32_t *process_pd = (uint32_t *)mmap(NULL, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    void *phys_addr = get_physaddr((void *)process_pd);
    
    //map first page directory for VGA text buffer
    process_pd[0] = virtual_page_directory[0];
    for(int i=KERNEL_PAGE_DIRECTORY_INDEX; i<1023; i++){
        process_pd[i] = virtual_page_directory[i];
    }
    //for being able to write in 0xb8000 VGA memory
    // process_pd[0] = virtual_page_directory[0];
    process_pd[1023] = (uint32_t)phys_addr | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    kernel_page_directory = virtual_page_directory[1023];
    
    

    return phys_addr;
}

extern void kpanic();

int load_process_ELF(task_struct *process, DISK *disk, const char* path, int isExecve){
    process->ELFfile = mmap(NULL, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    add_memory_mapping(process, (uint32_t)process->ELFfile, PROT_READ|PROT_WRITE, PAGE_SIZE, 0, "[exec ELFfile]");
    
    
    FAT_File *fd = FAT_Open(disk, path);

    if(fd == NULL){
        printf("File %s not found!\n", path);
        return -1;
    }

    if(ELF_parseFile(disk, fd, process->ELFfile)){
        return -1;
    }
    
    if(ELF_load(disk, fd, process->ELFfile, process)){
        return -1;
    }

    FAT_Close(disk, fd);

    process->eip = (void *)process->ELFfile->header->ProgramEntryPosition;

    fd = FAT_Open(disk, "/usr/lib/ld.so");

    if(fd == NULL){
        printf("Could not find or open PH_INTERP!\n");
        return -1;
    }

    dynamicLoader = mmap(NULL, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);

    add_memory_mapping(process, (uint32_t)dynamicLoader, PROT_READ | PROT_WRITE, PAGE_SIZE, 0, "[ld.so ELFfile]");

    if(dynamicLoader == NULL){
        printf("Failed to malloc memory for dynamic loader");
        return -1;
    }

    if(ELF_parseFile(disk, fd, dynamicLoader)){
        return -1;
    }

    if(ELF_load(disk, fd, dynamicLoader, process)){
        return -1;
    }
    
    FAT_Close(disk, fd);

    return 0;
}

void map_stack(task_struct *process){
    void *stack_addr = mmap((void *)0xbff00000, 8*PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    process->esp0 = stack_addr + 8*PAGE_SIZE;
    process->esp = stack_addr + 0xfcc + 7*PAGE_SIZE;
    uint32_t *buffer = (uint32_t *)(stack_addr + 0xfdc + 7*PAGE_SIZE);
    *buffer = (uint32_t)task_entry;
    add_memory_mapping(process, (uint32_t)stack_addr, PROT_READ | PROT_WRITE, 8*PAGE_SIZE, 0, "stack");
}

void reset_stack(task_struct *process){
    process->esp = process->esp0 - 0x34;
    uint32_t *buffer = (uint32_t *)(process->esp + 0x10);
    *buffer = (uint32_t)task_entry;
}

#define NO_EXECVE 0
#define IS_EXECVE 1

int change_to_new_executable(task_struct *process, DISK *disk, const char *path){
    process->number_of_mappings = 0;
    if(load_process_ELF(process, disk, path, NO_EXECVE) != 0){
        printf("Failed to load ELF file!\n");
        return -1;
    }
    reset_stack(process);
    asm volatile(
        "mov %0, %%esp \n\t" 
        "pop %%ebp \n\t"
        "pop %%edi \n\t"
        "pop %%esi \n\t"
        "pop %%ebx \n\t"
        "ret"
        :: "r"(process->esp));
    return 0;
}

int load_process(task_struct *process, DISK *disk, const char* path){
    void *process_pd = map_page_directory_kernel();
    write_cr3((uint32_t)process_pd);
    process->cr3 = process_pd; 

    FAT_File *stdin = FAT_Open(disk, "/dev/fd/0");
    if(stdin == NULL){
        printf("Couldn't open /dev/fd/0\n");
        return -1;
    }

    FAT_File *stdout = FAT_Open(disk, "/dev/fd/1");
    if(stdout == NULL){
        printf("Couldn't open /dev/fd/1\n");
        return -1;
    }

    FAT_File *stderr = FAT_Open(disk, "/dev/fd/2");
    if(stderr == NULL){
        printf("Couldn't open /dev/fd/2\n");
        return -1;
    }

    process->fd[0] = stdin;
    process->fd[1] = stdout;
    process->fd[2] = stderr;
    process->openedFiles = 3;

    if(load_process_ELF(process, disk, path, NO_EXECVE) != 0){
        printf("Failed to load ELF file!\n");
        return -1;
    }

    map_stack(process);

    write_cr3(kernel_page_directory & ~0xfff);
    
    process->pid = count_process_id++;
    process->state = TASK_RUNNING;
    return 0;
}
