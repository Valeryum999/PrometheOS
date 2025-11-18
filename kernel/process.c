#include <kernel/process.h>

uint32_t kernel_page_directory;
uint32_t count_process_id = 0;
ELF32_File *dynamicLoader;

extern void __attribute__((naked)) task_entry(void *first_eip);

void *map_page_directory_kernel(){
    uint32_t *process_pd = (uint32_t *)mmap(NULL, PAGE_SIZE, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER, MAP_ANONYMOUS, -1, 0);
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

void write_cr3(uint32_t pd){
    __asm__ volatile("mov %0, %%cr3" :: "r"(pd));
}

int load_process_ELF(task_struct *process, DISK *disk, const char* path, void *page_directory){
    process->cr3 = (void *)page_directory; 
    process->ELFfile = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
    
    FAT_File *fd = FAT_Open(disk, path);

    if(fd == NULL){
        printf("File %s not found!\n", path);
        return -1;
    }

    if(ELF_parseFile(disk, fd, process->ELFfile)){
        return -1;
    }
    
    if(ELF_load(disk, fd, process->ELFfile)){
        return -1;
    }

    FAT_Close(disk, fd);

    fd = FAT_Open(disk, "/usr/lib/ld.so");

    if(fd == NULL){
        printf("Could not find or open PH_INTERP!\n");
        return -1;
    }

    dynamicLoader = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);

    if(dynamicLoader == NULL){
        printf("Failed to malloc memory for dynamic loader");
        return -1;
    }

    if(ELF_parseFile(disk, fd, dynamicLoader)){
        return -1;
    }

    if(ELF_load(disk, fd, dynamicLoader)){
        return -1;
    }
    
    FAT_Close(disk, fd);

    process->eip = (void *)process->ELFfile->header->ProgramEntryPosition;
    return 0;
}

void map_stack(task_struct *process){
    void *stack = mmap(NULL, 4*PAGE_SIZE, PAGE_WRITABLE | PAGE_USER | PAGE_PRESENT, MAP_ANONYMOUS, -1, 0);
    process->esp = stack + 0xfcc + 3*PAGE_SIZE;
    process->esp0 = stack + 4*PAGE_SIZE;
    uint32_t *buffer = (uint32_t *)(stack + 0xfdc + 3*PAGE_SIZE);
    *buffer = (uint32_t)task_entry;
}

int load_process(task_struct *process, DISK *disk, const char* path){
    void *process_pd = map_page_directory_kernel();
    write_cr3((uint32_t)process_pd);

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

    if(load_process_ELF(process, disk, path, process_pd) != 0){
        printf("Failed to load ELF file!\n");
        return -1;
    }

    map_stack(process);

    write_cr3(kernel_page_directory & ~0xfff);
    
    process->id = count_process_id++;
    return 0;
}
