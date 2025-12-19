#include <kernel/syscall.h>
#include <flanterm.h>
#include <kernel/io.h>
#include <kernel/logging.h>

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
    const unsigned char* bytes = (const unsigned char*) regs->ebx;
	while(*bytes){
		i686_outb(0xe9, *bytes);
        bytes++;
    }
    return 0;
}

extern void afterFork(task_struct *child);
extern uint32_t kernel_heap_alloc;
extern uint32_t random_alloc;
extern stack_t stack;

uint32_t ExitHandler(Registers *regs){
    // printf("Exited with status code %d\n", regs->ebx);

    void *phys_addr;
    //print_memory_mappings(current_task_PCB);

    // free all the page frames allocated in this process' address space
    for(size_t i=0; i<current_task_PCB->number_of_mappings; i++){
        void *start_addr = (void *)current_task_PCB->vmmap[i].start_addr;
        size_t page_frames = (current_task_PCB->vmmap[i].end_addr - current_task_PCB->vmmap[i].start_addr) / PAGE_SIZE;
        // printf("Freeing %d page_frames @ %x\n", page_frames, start_addr);
        for(int j=0; j<page_frames; j++){
            phys_addr = get_physaddr(start_addr + PAGE_SIZE * j);
            if(phys_addr == 0){
                error_print("pushing NULL to stack allocator");
                kpanic();
            }
            free(phys_addr);
        }
    }

    random_alloc = 0xba02000;
    kernel_heap_alloc = 0xc2102000;


    remove_running_process_from_runqueue();
    // finally, switch to the next process in queue
    schedule();
    // afterFork(current_task_PCB->next);

    // should not return
    return 0;
}

uint32_t ForkHandler(Registers *regs){
    // TODO ASAP all of these mappings should be in the kernel heap
    task_struct *child_task = mmap((void *)kernel_heap_alloc, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    child_task->number_of_mappings = 0;
    add_memory_mapping(child_task, kernel_heap_alloc, PROT_READ|PROT_WRITE, PAGE_SIZE, 0, "[kheap]");
    kernel_heap_alloc += PAGE_SIZE;

    uint32_t *child_pd = map_and_zero_page((void *)kernel_heap_alloc, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    add_memory_mapping(child_task, kernel_heap_alloc, PROT_READ|PROT_WRITE, PAGE_SIZE, 0, "[kheap]");
    kernel_heap_alloc += PAGE_SIZE;

    //copy the parent's pd
    for(size_t i=0; i<1023; i++){
        child_pd[i] = virtual_page_directory[i];
    }

    //set every PTE as read-only to COW only when needed
    for(size_t i=1; i<768; i++){
        if(!(virtual_page_directory[i] & PAGE_PRESENT)){
            continue;
        }
        uint32_t *page_table = (uint32_t *)(0xffc00000 + i * PAGE_SIZE);
        uint32_t *child_page_table = map_and_zero_page((void *)kernel_heap_alloc, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
        add_memory_mapping(child_task, kernel_heap_alloc, PROT_READ|PROT_WRITE, PAGE_SIZE, 0, "[kheap]");
        kernel_heap_alloc += PAGE_SIZE;

        child_pd[i] = (uint32_t)get_physaddr(child_page_table) | PAGE_USER | PAGE_WRITABLE | PAGE_PRESENT;
        for(size_t j=0; j<1024; j++){
            // page_table[j] &= ~PAGE_WRITABLE;
            child_page_table[j] = page_table[j] & ~PAGE_WRITABLE;
        }
    }

    // TODO should unmap the child_pd from the parent's pd

    uint32_t child_pd_phys_addr = (uint32_t)get_physaddr((void *)child_pd);
    child_pd[1023] = child_pd_phys_addr | PAGE_USER | PAGE_WRITABLE | PAGE_PRESENT;
    // should be in kernel heap
    child_task->cr3 = (void *)child_pd_phys_addr;
    child_task->esp0 = mmap((void *)kernel_heap_alloc, PAGE_SIZE, PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
    add_memory_mapping(child_task, kernel_heap_alloc, PROT_READ|PROT_WRITE, PAGE_SIZE, 0, "[kheap]");
    kernel_heap_alloc += PAGE_SIZE;
    child_task->esp0 += PAGE_SIZE;
    child_task->esp = (void *)regs->kern_esp;
    child_task->eip = (void *)regs->eip;
    //should the child inherit all the file descriptors?
    
    child_task->fd[0] = current_task_PCB->fd[0]; 
    child_task->fd[1] = current_task_PCB->fd[1]; 
    child_task->fd[2] = current_task_PCB->fd[2];
    FAT_IncreaseRefcount(current_task_PCB->fd[0]);
    FAT_IncreaseRefcount(current_task_PCB->fd[1]);
    FAT_IncreaseRefcount(current_task_PCB->fd[2]);
    for(int i=3; i<10; i++)
        child_task->fd[i] = current_task_PCB->fd[i];
    // child_task->openedFiles = current_task_PCB->openedFiles;

    
    // child_task->number_of_mappings = current_task_PCB->number_of_mappings;
    // for(size_t i=0; i<child_task->number_of_mappings; i++){
    //     child_task->vmmap[i] = current_task_PCB->vmmap[i]; 
    // }

    child_task->ppid = current_task_PCB->pid;
    child_task->pid = count_process_id++;
    child_task->state = TASK_RUNNING;
    current_task_PCB->child_pid = child_task->pid;
    //add child process to the runqueue
    add_process_to_schedule(child_task);
    //switch to the child process
    afterFork(child_task);
    //the parent should resume from here
    return (child_task->pid == current_task_PCB->pid) ? 0 : child_task->pid;
}

uint32_t ReadHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    char *buf = (char *)regs->ecx;
    size_t len = regs->edx;
    i686_EnableInterrupts();
    while(fd == STDIN && current_task_PCB->fd[fd]->Size == 0){}
    if(fd == STDIN){
        uint32_t result = FAT_Read(disk, current_task_PCB->fd[fd], len, buf);
        current_task_PCB->fd[fd]->Position = 0;
        current_task_PCB->fd[fd]->Size = 0;
        return result;
    }
    i686_DisableInterrupts();
    return FAT_Read(disk, current_task_PCB->fd[fd], len, buf);
}

extern struct flanterm_context *ft_ctx;
#include <kernel/io.h>

uint32_t WriteHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    const char *buf = (const char *)regs->ecx;
    size_t len = regs->edx;
    //little workaround for the moment
    //TODO replace this with actual synchronization between /dev/fd/1 and tty
    if(fd == STDOUT || fd == STDERR){
        const unsigned char* bytes = (const unsigned char*) buf;
        for (size_t i = 0; i < len; i++)
            i686_outb(0xe9, bytes[i]);
        flanterm_write(ft_ctx, buf, len);
    }
    return FAT_Write(disk, current_task_PCB->fd[fd], len, buf);
}

uint32_t OpenHandler(Registers *regs){
    const char *path = (const char *)regs->ebx;
    // current_task_PCB->fd[current_task_PCB->openedFiles] = FAT_Open(disk, path);
    // return current_task_PCB->openedFiles++ + 3; //to reserve 0,1,2 for stdin, stdout, stderr
    return -1; //unimplemented
}

uint32_t OpenAtHandler(Registers *regs){
    int dfd = (int)regs->ebx;
    const char *path = (const char *)regs->ecx;
    if(verbose)
        printf("OPENAT: path is %s\n",path);
    FAT_File *result = FAT_Open(disk, path);
    if(result == NULL){
        return -1;
    }
    uint32_t fd = -1;
    for(size_t i=0; i<10; i++){
        if(current_task_PCB->fd[i] == NULL){
            current_task_PCB->fd[i] = result;
            fd = i;
            break;
        }
    }
    if(fd == -1){
        printf("Openat: couldn't find free fd!\n");
    }
    return fd;
}

uint32_t CloseHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    if(verbose)
        printf("CLOSE: closing %d\n", fd);
    FAT_Close(disk, current_task_PCB->fd[fd]);
    //what to do with dangling fd?
    current_task_PCB->fd[fd] = NULL;
    return 0;
}

uint32_t WaitPidHandler(Registers *regs){
    uint32_t pid = regs->ebx;
    // printf("WAITPID syscall pid: %x\n", pid);
    current_task_PCB->state = TASK_INTERRUPTIBLE;
    // deactivate_current_running_task();
    // schedule();
    return current_task_PCB->child_pid;
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
    if(verbose)
        printf("EXECVE: %s with argv: %x and envp: %x\n", path, argv, envp);
    debugIsExecve = 1;


    // print_memory_mappings(current_task_PCB);
    // free all the page frames allocated in this process' address space
    // for(size_t i=0; i<current_task_PCB->number_of_mappings; i++){
    //     void *start_addr = (void *)current_task_PCB->vmmap[i].start_addr;
    //     size_t page_frames = (current_task_PCB->vmmap[i].end_addr - current_task_PCB->vmmap[i].start_addr) / PAGE_SIZE;
    //     printf("EXECVE: Freeing %d page_frames @ %x\n", page_frames, start_addr);
    //     for(size_t j=0; j<page_frames; j++){
    //         void *phys_addr = get_physaddr(start_addr + PAGE_SIZE * j);
    //         if(phys_addr == NULL){
    //             kpanic();
    //         }
    //         free(phys_addr);
    //     }
    // }
    current_task_PCB->number_of_mappings = 0;
    // int argc = 0;
    // if(argv){
    //     while(argv[argc])
    //         argc++;
    // }

    // int envc = 0;
    // if(envp){
    //     while(envp[envc]){
    //         envc++;
    //     }
    // }

    // const char *copy_argv[argc+1];
    // const char *copy_envp[envc+1];

    // char *copy_argv_and_envp = mmap((void *)kernel_heap_alloc, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
    // add_memory_mapping(current_task_PCB, kernel_heap_alloc, PROT_READ | PROT_WRITE, PAGE_SIZE, 0, "[kheap]");
    // kernel_heap_alloc += PAGE_SIZE;
    // int i = 0;
    // while(i < argc){
    //     strcpy(copy_argv_and_envp, argv[i]);
    //     copy_argv[i] = copy_argv_and_envp;
    //     copy_argv_and_envp += strlen(argv[i]) + 1;
    //     i++;
    // }
    // copy_argv[argc] = NULL;
    // i = 0;
    // while(i < envc){
    //     strcpy(copy_argv_and_envp, envp[i]);
    //     copy_envp[i] = copy_argv_and_envp;
    //     copy_argv_and_envp += strlen(envp[i]) + 1;
    //     i++;
    // }
    // copy_envp[envc] = NULL;

    change_to_new_executable(current_task_PCB, disk, path, argv, envp);

    //if returns it's an error
    return -1;
}

uint32_t LSeekHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    if(current_task_PCB->fd[fd] == NULL){
        // printf("LSEEK: fd points to a non-existent file!\n");
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
    if(verbose)
        printf("GETUID is a stub\n");
    return 0;
}

uint32_t GetGidHandler(Registers *regs){
    if(verbose)
        printf("GETGID is a stub\n");
    return 0;
}

uint32_t GetEUidHandler(Registers *regs){
    if(verbose)
        printf("GETEUID is a stub\n");
    return 0;
}

uint32_t GetEGidHandler(Registers *regs){
    if(verbose)
        printf("GETEGID is a stub\n");
    return 0;
}

uint32_t GetPGidHandler(Registers *regs){
    //GetPGidHandler stubbed for now
    return 0;
}

uint32_t KillHandler(Registers *regs){
    uint32_t pid = regs->ebx;
    int signal = (int)regs->ecx;
    if(verbose) 
        printf("Kill handler pid: %x signal: %x\n", pid, signal);
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

#define AT_EMPTY_PATH 0x1000

uint32_t FStatAt64Handler(Registers *regs){
    uint32_t     dfd     = regs->ebx;
    const char*  path    = (const char*)regs->ecx;
    struct stat* statbuf = (struct stat*)regs->edx;
    int          flags   = (int)regs->esi;

    FAT_File *fd;

    if(flags & AT_EMPTY_PATH){
        fd = current_task_PCB->fd[dfd];
    } else {
        fd = FAT_Open(disk, path);
    }

    if(fd == NULL)
        return -1;

    uint32_t result = FAT_StatAt(disk, fd, flags, statbuf);

    if(!(flags & AT_EMPTY_PATH))
        FAT_Close(disk, fd);

    return result;
}

uint32_t FAccessAtHandler(Registers *regs){
    uint32_t dirfd = regs->ebx;
    const char *path = (const char *)regs->ecx;
    int mode = (int)regs->edx;
    
    FAT_File *fd = FAT_Open(disk, path);
    uint32_t result = 0;
    if(fd == NULL){
        result = -1;
    } else {
        FAT_Close(disk, fd);
    }

    return result;
}

uint32_t MMAPHandler(Registers *regs){
    void *virtual_addr = (void *)regs->ebx;
    size_t size = (size_t)regs->ecx;
    int prot = (int)regs->edx;
    int flags = (int)regs->esi;
    int fd = (int)regs->edi;
    uint32_t offset = regs->ebp;
    uint32_t result = (uint32_t)map_and_zero_page(virtual_addr, size, PAGE_WRITABLE, flags, fd, offset);
    add_memory_mapping(current_task_PCB, result, prot, size, offset, "[heap]");
    if(verbose)
        printf("mmap @ %x + %x with prot %x flags %x fd %x offset %x\n",
                result,
                size,
                prot,
                flags,
                fd,
                offset);
    return result;
}

uint32_t MUNMAPHandler(Registers *regs){
    void *virtual_addr = (void *)regs->ebx;
    size_t size = (size_t)regs->ecx;
    if(verbose)
        printf("munmap @ %x + %x\n",
                virtual_addr,
                size);
    uint32_t result = (uint32_t)munmap(virtual_addr, size);
    remove_memory_mapping(current_task_PCB, (uint32_t)virtual_addr, size);
    return result;
}

uint32_t MProtectHandler(Registers *regs){
    void *start = (void *)regs->ebx;
    size_t size = (size_t)regs->ecx;
    uint32_t prot = regs->edx;
    
    return mprotect(start, size, prot);
}

uint32_t ArchPRCTLHandler(Registers *regs){
    current_task_PCB->gs_base = regs->ebx;
    return change_gs_base(current_task_PCB->gs_base);
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
    if(verbose)
        printf("IOCTL %s fd: %d cmd: 0x%x arg: 0x%x is a stub\n", ioctl_cmds[cmd-TCGETS], fd, cmd, arg);
    switch(cmd){
        case TCGETS:
            // struct termios *term = (struct termios *)arg;
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
    if(verbose)
        printf("FCNTL is a stub, fd: %d cmd: %x arg: %x\n", fd, cmd, arg);
    return 0;
}

#define VOLUME_ID 0x08
#define DIRECTORY 0x10
#define ARCHIVE   0x20

uint32_t ReadDirEntsHandler(Registers *regs){
    uint32_t dfd = regs->ebx;
    linux_dirent *dirEntries = (linux_dirent *)regs->ecx;
    uint32_t count = regs->edx;
    if(verbose)
        printf("SYSCALL READDIRENTS fd: %x dirent: %x count: %x\n", dfd, dirEntries, count);
    //TODO fix
    uint32_t bytes_read = 0;
    FAT_DirectoryEntry dirEntry;
    size_t i = 0;
    FAT_File *fd = current_task_PCB->fd[dfd];

    while(bytes_read < count && FAT_ReadEntry(disk, fd, &dirEntry) && dirEntry.Name[0]){
        dirEntries[i].d_ino = i;
        dirEntries[i].d_off = i;
        FAT_FATfilename_to_filename(dirEntry.Name, dirEntries[i].d_name);
        if(dirEntry.Attributes == ARCHIVE){
            dirEntries[i].d_type = DT_REG;
            // printf("%s is a regular file!\n", dirEntries[i].d_name);
        }
        else if(dirEntry.Attributes == DIRECTORY){
            dirEntries[i].d_type = DT_DIR;
            // printf("%s is a directory!\n", dirEntries[i].d_name);
        }
        else{
            dirEntries[i].d_type = DT_UNKNOWN;
            // printf("%s is unknown!\n", dirEntries[i].d_name);
            continue;
        }
        dirEntries[i].d_reclen = sizeof(linux_dirent);//offsetof(linux_dirent, d_name) + 11;
        i++;
        bytes_read += sizeof(linux_dirent);//offsetof(linux_dirent, d_name) + 12;
    }

    return bytes_read;
}

uint32_t GenericSyscall(Registers *regs){
    if(verbose)
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
        case GETDENTS:
            return ReadDirEntsHandler(regs);
        case STAT:
            if(verbose)
                printf("Syscall: STAT\n");
            return StatHandler(regs);
        case FSTATAT64:
            if(verbose)
                printf("Syscall: FSTATAT64\n");
            return FStatAt64Handler(regs);
        case FACCESAT:
            return FAccessAtHandler(regs);
        case MMAP:
            return MMAPHandler(regs);
        case MUNMAP:
            return MUNMAPHandler(regs);
        case MPROTECT:
            if(verbose)
                printf("Syscall: MPROTECT\n");
            return MProtectHandler(regs);
        case UNAME:
            if(verbose)
                printf("Syscall: UNAME\n");
            return UnameHandler(regs);
        case GETPGID:
            if(verbose)
                printf("Syscall: GETPGID\n");
            return GetPGidHandler(regs);
        case ARCH_PRCTL:
            if(verbose)
                printf("Syscall: ARCH_PRCTL\n");
            return ArchPRCTLHandler(regs);
        default:
            return GenericSyscall(regs);
            // kpanic();
    }
}