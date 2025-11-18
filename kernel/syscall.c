#include <kernel/syscall.h>

extern void kpanic();
extern DISK *disk;
extern task_struct *current_task_PCB;
int verbose = 0;

#define STDIN 0
#define STDOUT 1
#define STDERR 2

uint32_t MLibcLog(Registers *regs){
    printf("%s\n", regs->ebx);
    return 0;
}

uint32_t ExitHandler(Registers *regs){
    printf("Exited with status code %d\n", regs->ebx);
    task_struct *previous_task = current_task_PCB;
    while(previous_task->next != current_task_PCB){
        previous_task = previous_task->next;
    }
    previous_task->next = current_task_PCB->next;
    switch_to_task(current_task_PCB->next);
    return 0;
}

uint32_t ForkHandler(Registers *regs){
    // task_struct child = load_process(current_task_PCB->ELFfile);
    // child.eip = (void *)regs->eip;
    // add_process_to_schedule(&child);
    return 0;
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
    return current_task_PCB->openedFiles++; //to reserve 0,1,2 for stdin, stdout, stderr
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
    printf("Wait PID Handler, for now stubbed\n");
    return 0;
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

uint32_t ExecveHandler(Registers *regs){
    const char *filename = (const char *)regs->ebx;
    const char **argv = (const char **)regs->ecx;
    const char **envp = (const char **)regs->edx;
    printf("Execve handler, for now stubbed\n");
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
    return current_task_PCB->id;
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
    if(verbose){
        printf("mmap @ %x + %x with prot %x flags %x fd %x offset %x\n",
                virtual_addr,
                size,
                prot,
                flags,
                fd,
                offset);
    }
    return (uint32_t)mmap(virtual_addr, size, prot, flags, fd, offset);
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
    if(regs->eax < 92 && regs->eax != 0){
        if(*syscall_strings[regs->eax]){
            printf("Syscall: %s\n", syscall_strings[regs->eax]);
        } else {
            printf("Undefined syscall: %d\n", regs->eax);
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