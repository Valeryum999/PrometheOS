#include <kernel/syscall.h>

extern void kpanic();
extern DISK *disk;
extern task_struct *current_task_PCB;
extern ELF32_File *dynamicLoader;
int verbose = 0;
extern uint32_t count_process_id;

#define STDIN 0
#define STDOUT 1
#define STDERR 2

void dump_memory(void *addr){
    uint8_t *dump = (uint8_t *) addr;
    for(size_t i=0; i<1; i++){
        for(size_t j=0; j<16; j++){
            if(dump[i*16+j] < 16){
                printf("0");
            }
            printf("%x",dump[i*16+j]);
        }
        printf("\n");
    }
}

uint32_t MLibcLog(Registers *regs){
    printf("%s\n", regs->ebx);
    return 0;
}

uint32_t ExitHandler(Registers *regs){
    printf("Exited with status code %d\n", regs->ebx);

    // free all the page frames allocated in this process' address space
    for(size_t i=0; i<current_task_PCB->number_of_mappings; i++){
        void *start_addr = get_physaddr((void *)current_task_PCB->vmmap[i].start_addr);
        size_t page_frames = (current_task_PCB->vmmap[i].end_addr - current_task_PCB->vmmap[i].start_addr) / PAGE_SIZE;
        free_page_frames(start_addr, page_frames);
    }

    // free curr executable ELFfile struct
    free(get_physaddr((void *)current_task_PCB->ELFfile));

    // free curr executable task_struct
    free(get_physaddr((void *)current_task_PCB));

    // free ld.so ELFfile struct
    free(get_physaddr((void *)dynamicLoader));

    // free the page directory thanks to recursive paging
    free(get_physaddr(virtual_page_directory));

    // finally, switch to the next process in queue
    schedule();

    // does not return
    return 0;
}

extern void afterFork(task_struct *child);

uint32_t ForkHandler(Registers *regs){
    printf("FORK: idk forking ig\n");
    // TODO ASAP all of these mappings should be in the kernel heap
    uint32_t *child_pd = mmap(NULL, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    for(size_t i=0; i<256; i++){
        child_pd[i] = virtual_page_directory[i];
    }

    for(size_t i=256; i<768; i++){
        // copy all memory mappings as read only and when 
        // a page fault happens do copy on write
        child_pd[i] = virtual_page_directory[i] & ~PAGE_WRITABLE;
    }

    for(size_t i=768; i<1023; i++){
        child_pd[i] = virtual_page_directory[i];
    }
    // TODO should unmap the child_pd from the parent's pd

    uint32_t child_pd_phys_addr = (uint32_t)get_physaddr((void *)child_pd);
    child_pd[1023] = child_pd_phys_addr | PAGE_WRITABLE | PAGE_PRESENT;
    // should be in kernel heap
    task_struct *child_task = mmap(NULL, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    child_task->cr3 = (void *)child_pd_phys_addr;
    child_task->esp = (void *)regs->kern_esp;
    child_task->esp0 = current_task_PCB->esp0;
    child_task->eip = (void *)regs->eip;

    //should the child inherit all the file descriptors?
    child_task->fd[0] = current_task_PCB->fd[0]; 
    child_task->fd[1] = current_task_PCB->fd[1]; 
    child_task->fd[2] = current_task_PCB->fd[2]; 
    child_task->openedFiles = current_task_PCB->openedFiles;

    child_task->number_of_mappings = current_task_PCB->number_of_mappings;
    for(size_t i=0; i<child_task->number_of_mappings; i++){
        child_task->vmmap[i] = current_task_PCB->vmmap[i]; 
    }

    child_task->ppid = current_task_PCB->pid;
    child_task->pid = count_process_id++;
    child_task->state = TASK_RUNNING;
    //switch to the child process
    add_process_to_schedule(child_task);
    afterFork(child_task);
    return (child_task->pid == current_task_PCB->pid) ? 0 : child_task->pid;
}

uint32_t ReadHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    char *buf = (char *)regs->ecx;
    size_t len = regs->edx;
    return FAT_Read(disk, current_task_PCB->fd[fd], len, buf);
}

uint32_t WriteHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    const char *buf = (const char *)regs->ecx;
    size_t len = regs->edx;
    //little workaround for the moment
    //TODO replace this with actual synchronization between /dev/fd/1 and tty
    if(fd == STDOUT || fd == STDERR)
        printf("%s", buf);
    return FAT_Write(disk, current_task_PCB->fd[fd], len, buf);
}

uint32_t OpenHandler(Registers *regs){
    const char *path = (const char *)regs->ebx;
    current_task_PCB->fd[current_task_PCB->openedFiles] = FAT_Open(disk, path);
    return current_task_PCB->openedFiles++ + 3; //to reserve 0,1,2 for stdin, stdout, stderr
}

uint32_t OpenAtHandler(Registers *regs){
    int dfd = (int)regs->ebx;
    const char *path = (const char *)regs->ecx;
    printf("OPENAT: path is %s\n",path);
    FAT_File *result = FAT_Open(disk, path);
    if(result == NULL){
        return -1;
    }
    current_task_PCB->fd[current_task_PCB->openedFiles] = result;
    uint32_t fd = current_task_PCB->openedFiles++;
    printf("OPENAT: returning fd %d\n",fd);
    return fd; //to reserve 0,1,2 for stdin, stdout, stderr
}

uint32_t CloseHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    printf("CLOSE: closing %d\n", fd);
    FAT_Close(disk, current_task_PCB->fd[fd]);
    //what to do with dangling fd?
    //current_task_PCB->fd[fd] = NULL;
    return 0;
}

uint32_t WaitPidHandler(Registers *regs){
    uint32_t pid = regs->ebx;
    current_task_PCB->state = TASK_INTERRUPTIBLE;
    deactivate_current_running_task();
    schedule();
    return pid;
}

uint32_t LinkHandler(Registers *regs){
    const char *old_name = (const char *)regs->ebx;
    const char *new_name = (const char *)regs->ecx;
    return FAT_CopyFile(disk, old_name, new_name);
}

uint32_t UnlinkHandler(Registers *regs){
    const char *pathname = (const char *)regs->ebx;
    printf("Unlink handler, for now stubbed\n");
    return 0;
}

int debugIsExecve = 0;

uint32_t ExecveHandler(Registers *regs){
    const char *path = (const char *)regs->ebx;
    const char **argv = (const char **)regs->ecx;
    const char **envp = (const char **)regs->edx;

    printf("EXECVE: %s with argv: %x and envp: %x\n", path, argv, envp);
    debugIsExecve = 1;

    // free all the page frames allocated in this process' address space
    for(size_t i=0; i<current_task_PCB->number_of_mappings; i++){
        if(!strcmp(current_task_PCB->vmmap[i].path, "stack"))
            continue;
        void *start_addr = (void *)current_task_PCB->vmmap[i].start_addr;
        size_t page_frames = (current_task_PCB->vmmap[i].end_addr - current_task_PCB->vmmap[i].start_addr) / PAGE_SIZE;
        for(size_t j=0; j<page_frames; j++){
            free(get_physaddr(start_addr + PAGE_SIZE * j));
        }
    }
    // current_task_PCB->vmmap[0].start_addr = 
    //should not return
    change_to_new_executable(current_task_PCB, disk, path);

    //if returns it's an error
    return 0;
}

uint32_t LSeekHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    if(current_task_PCB->fd[fd] == NULL){
        printf("LSEEK: fd points to a non-existent file!\n");
        return -1;
    }
    uint32_t offset = regs->ecx;
    uint32_t whence = regs->edx;
    return FAT_LSeek(disk, current_task_PCB->fd[fd], offset, whence);
}

uint32_t GetPidHandler(Registers *regs){
    return current_task_PCB->pid;
}

uint32_t GetUidHandler(Registers *regs){
    printf("GETUID is a stub\n");
    return 0;
}

uint32_t GetGidHandler(Registers *regs){
    printf("GETGID is a stub\n");
    return 0;
}

uint32_t GetEUidHandler(Registers *regs){
    printf("GETEUID is a stub\n");
    return 0;
}

uint32_t GetEGidHandler(Registers *regs){
    printf("GETEGID is a stub\n");
    return 0;
}

uint32_t GetPGidHandler(Registers *regs){
    //GetPGidHandler stubbed for now
    return 0;
}

uint32_t KillHandler(Registers *regs){
    printf("Kill handler, for now stubbed\n");
    return 0;
}

uint32_t TimesHandler(Registers *regs){
    printf("Times handler, for now stubbed\n");
    return 0;
}

uint32_t StatHandler(Registers *regs){
    printf("Stat handler, for now stubbed\n");
    //FAT_File *root_dir = FAT_Open(disk, "/");
    return 0;
}

uint32_t FStatAt64Handler(Registers *regs){
    uint32_t     dfd     = regs->ebx;
    const char*  path    = (const char*)regs->ecx;
    struct stat* statbuf = (struct stat*)regs->edx;
    int          flags   = (int)regs->esi;

    FAT_File *fd = FAT_Open(disk, path);

    if(fd == NULL)
        return -1;

    return FAT_StatAt(disk, path, flags, statbuf);
}

uint32_t MMAPHandler(Registers *regs){
    void *virtual_addr = (void *)regs->ebx;
    size_t size = (size_t)regs->ecx;
    int prot = (int)regs->edx;
    int flags = (int)regs->esi;
    int fd = (int)regs->edi;
    uint32_t offset = regs->ebp;
   
        printf("mmap @ %x + %x with prot %x flags %x fd %x offset %x\n",
                virtual_addr,
                size,
                prot,
                flags,
                fd,
                offset);
    uint32_t result = (uint32_t)mmap(virtual_addr, size, PAGE_WRITABLE, flags, fd, offset);
    add_memory_mapping(current_task_PCB, result, prot, size, offset, "[heap]");
    return result;
}

uint32_t MProtectHandler(Registers *regs){
    void *start = (void *)regs->ebx;
    size_t size = (size_t)regs->ecx;
    uint32_t prot = regs->edx;
    
    return mprotect(start, size, prot);
}

uint32_t ArchPRCTLHandler(Registers *regs){
    return change_gs_base(regs->ebx);
}

uint32_t UnameHandler(Registers *regs){
    struct utsname* info = (struct utsname*)regs->ebx;
    strcpy(info->sysname, "prometheos");
    strcpy(info->nodename, "user");
    strcpy(info->release, "0.0.1");
    strcpy(info->version, "0.0.1");
    strcpy(info->machine, "");
    strcpy(info->domainname, "");
    return 0;
}

uint32_t IOCTLHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    uint32_t cmd = regs->ecx;
    uint32_t arg = regs->edx;
    printf("IOCTL %s fd: %d cmd: 0x%x arg: 0x%x is a stub\n", ioctl_cmds[cmd-TCGETS], fd, cmd, arg);
    switch(cmd){
        case TCGETS:
            break;
        case TIOCGWINSZ:
            struct winsize *window = (struct winsize*)regs->edx;
            window->ws_row = 25;
            window->ws_col = 80;
            window->ws_xpixel = 1;
            window->ws_ypixel = 1;
            return 0;
    }
    return 0;
}

uint32_t FCNTLHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    uint32_t cmd = regs->ecx;
    uint32_t arg = regs->edx;
    printf("FCNTL is a stub, fd: %d cmd: %x arg: %x\n", fd, cmd, arg);
    return 0;
}

uint32_t GenericSyscall(Registers *regs){
    printf("Unhandled syscall: %d\n", regs->eax);
    return 0;
}

uint32_t SyscallHandler(Registers *regs){
    if(verbose){
        if(regs->eax < 92 && regs->eax != 0){
            if(*syscall_strings[regs->eax]){
                printf("Syscall: %s\n", syscall_strings[regs->eax]);
            } else {
                printf("Undefined syscall: %d\n", regs->eax);
            }
        }
    }
    switch(regs->eax){
        case 0:
            return MLibcLog(regs);
        case EXIT:
            return ExitHandler(regs);
        case FORK:
            return ForkHandler(regs);
        case READ:
            return ReadHandler(regs);
        case WRITE:
            return WriteHandler(regs);
        case OPEN:
            return OpenHandler(regs);
        case OPENAT:
            printf("Syscall: OPENAT\n");
            return OpenAtHandler(regs);
        case CLOSE:
            return CloseHandler(regs);
        case IOCTL:
            return IOCTLHandler(regs);
        case FCNTL:
            return FCNTLHandler(regs);
        case WAIT_PID:
            return WaitPidHandler(regs);
        case LINK:
            return LinkHandler(regs);
        case UNLINK:
            return WriteHandler(regs);
        case EXECVE:
            return ExecveHandler(regs);
        case LSEEK:
            return LSeekHandler(regs);
        case GETPID:
            return GetPidHandler(regs);
        case GETUID:
            return GetUidHandler(regs);
        case GETGID:
            return GetGidHandler(regs);
        case GETEUID:
            return GetEUidHandler(regs);
        case GETEGID:
            return GetEGidHandler(regs);
        case KILL:
            return KillHandler(regs);
        case TIMES:
            return TimesHandler(regs);
        case STAT:
            printf("Syscall: STAT\n");
            return StatHandler(regs);
        case FSTATAT64:
            printf("Syscall: FSTATAT64\n");
            return FStatAt64Handler(regs);
        case MMAP:
            return MMAPHandler(regs);
        case MPROTECT:
            printf("Syscall: MPROTECT\n");
            return MProtectHandler(regs);
        case UNAME:
            printf("Syscall: UNAME\n");
            return UnameHandler(regs);
        case GETPGID:
            printf("Syscall: GETPGID\n");
            return GetPGidHandler(regs);
        case ARCH_PRCTL:
            printf("Syscall: ARCH_PRCTL\n");
            return ArchPRCTLHandler(regs);
        default:
            return GenericSyscall(regs);
            // kpanic();
    }
}