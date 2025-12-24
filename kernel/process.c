#include <kernel/process.h>

uint32_t count_process_id = 0;
uint32_t kernel_page_directory;
ELF32_File *dynamicLoader;

extern void __attribute__((naked)) task_entry();

void write_cr3(uint32_t pd){
    __asm__ volatile("mov %0, %%cr3" :: "r"(pd));
}

extern uint32_t kernel_heap_alloc;

void *map_page_directory_kernel(task_struct *process){
    uint32_t *process_pd = map_and_zero_page((void *)kernel_heap_alloc, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    add_memory_mapping(process, kernel_heap_alloc, PROT_READ|PROT_WRITE, PAGE_SIZE, 0, "[kheap]");
    kernel_heap_alloc += PAGE_SIZE;
    void *phys_addr = get_physaddr((void *)process_pd);
    
    //map first page directory for low kernel code
    process_pd[0] = virtual_page_directory[0];
    for(int i=KERNEL_PAGE_DIRECTORY_INDEX; i<1023; i++){
        process_pd[i] = virtual_page_directory[i];
    }

    process_pd[1023] = (uint32_t)phys_addr | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    kernel_page_directory = virtual_page_directory[1023];
    
    return phys_addr;
}

extern void kpanic();

int load_process_ELF(task_struct *process, DISK *disk, const char* path, int isExecve){
    process->ELFfile = mmap(NULL, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    if(process->ELFfile == NULL){
        return -1;
    }
    add_memory_mapping(process, (uint32_t)process->ELFfile, PROT_READ|PROT_WRITE, PAGE_SIZE, 0, "[exec ELFfile]");
    
    
    FAT_File *fd = FAT_Open(disk, path);

    if(fd == NULL){
        printf("Process: File %s not found!\n", path);
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

void map_stack(task_struct *process, const char **argv, const char **envp){
    void *stack_addr = mmap((void *)0xbff00000, 8*PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    add_memory_mapping(process, (uint32_t)stack_addr, PROT_READ | PROT_WRITE, 8*PAGE_SIZE, 0, "stack");
    void *kstack_addr = mmap((void *)kernel_heap_alloc, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    add_memory_mapping(process, kernel_heap_alloc, PROT_READ|PROT_WRITE, PAGE_SIZE, 0, "[kheap]");
    kernel_heap_alloc += PAGE_SIZE;
    process->esp = stack_addr + 8*PAGE_SIZE;
    uint32_t *sp = (uint32_t *)process->esp - 0x10;
    sp--;
    *(sp--) = (uint32_t)envp;
    *(sp--) = (uint32_t)argv;
    sp--;
    *(sp--) = (uint32_t)task_entry;
    *(sp--) = 0;
    *(sp--) = 0;
    *(sp--) = 0;
    *(sp)   = 0;
    process->esp = sp;
    process->esp0 = kstack_addr + PAGE_SIZE;
}

void reset_stack(task_struct *process, const char **argv, const char **envp){
    process->esp = (void *)0xbff00000 + 7*PAGE_SIZE + 0xfcc;
    uint32_t *sp = (uint32_t *)(process->esp);
    sp--;
    *(sp--) = (uint32_t)envp;
    *(sp--) = (uint32_t)argv;
    sp--;
    *(sp) = (uint32_t)task_entry;
    process->esp = sp;
}

#define NO_EXECVE 0
#define IS_EXECVE 1

int change_to_new_executable(task_struct *process, DISK *disk, const char *path, const char **argv, const char **envp){
    // process->number_of_mappings = 0;
    if(load_process_ELF(process, disk, path, NO_EXECVE) != 0){
        printf("Failed to load ELF file!\n");
        return -1;
    }
    reset_stack(process, argv, envp);
    // reset_stack(process, argv, envp);
    // mov sp to task's kernel stack and ret to task_entry
    asm volatile(
        "mov %0, %%esp \n\t" 
        "xor %%ebp, %%ebp \n\t"
        "xor %%edi, %%edi \n\t"
        "xor %%esi, %%esi \n\t"
        "xor %%ebx, %%ebx \n\t"
        "ret"
        :: "r"(process->esp));

    return 0;
}

int load_process(task_struct *process, DISK *disk, const char* path, const char **argv, const char **envp){
    process->number_of_mappings = 0;
    void *process_pd = map_page_directory_kernel(process);
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

    if(load_process_ELF(process, disk, path, NO_EXECVE) != 0){
        printf("Failed to load ELF file!\n");
        return -1;
    }

    map_stack(process, argv, envp);

    write_cr3(kernel_page_directory & ~0xfff);
    
    process->pid = count_process_id++;
    process->state = TASK_RUNNING;
    return 0;
}
