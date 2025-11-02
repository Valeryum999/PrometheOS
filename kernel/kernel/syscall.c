#include <kernel/syscall.h>

extern void kpanic();
extern DISK *disk;
extern task_struct *current_task_PCB;

uint32_t ExitHandler(Registers *regs){
    printf("Exit handler\n");
    task_struct *previous_task = current_task_PCB;
    while(previous_task->next != current_task_PCB){
        previous_task = previous_task->next;
    }
    previous_task->next = current_task_PCB->next;
    switch_to_task(current_task_PCB->next);
    return 0;
}

uint32_t ForkHandler(Registers *regs){
    printf("Fork Handler, for now stubbed\n");
    return 0;
}

uint32_t ReadHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    if(fd < 3){
        // read from stdin, stout or stderr
        return 0;
    }
    fd -= 3;
    char *buf = (char *)regs->ecx;
    size_t len = regs->edx;
    return FAT_Read(disk, current_task_PCB->fd[fd], len, buf);
}

uint32_t WriteHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    if(fd < 3){
        // write to stdin, stout or stderr
        printf("%s",regs->ecx);
        return regs->edx;
    }
    fd -= 3;
    const char *buf = (const char *)regs->ecx;
    size_t len = regs->edx;
    return FAT_Write(disk, current_task_PCB->fd[fd], len, buf);
}

uint32_t OpenHandler(Registers *regs){
    const char *path = (const char *)regs->ebx;
    current_task_PCB->fd[current_task_PCB->openedFiles] = FAT_Open(disk, path);
    return current_task_PCB->openedFiles++ + 3; //to reserve 0,1,2 for stdin, stdout, stderr
}

uint32_t CloseHandler(Registers *regs){
    uint32_t fd = regs->ebx;
    if(fd < 3){
        // can't close stdin stdout stderr?
        return 0;
    }
    fd -= 3;
    FAT_Close(disk, current_task_PCB->fd[fd]);
    //what to do with dangling fd?
    return 0;
}

uint32_t WaitPidHandler(Registers *regs){
    printf("Wait PID Handler, for now stubbed\n");
    return 0;
}

uint32_t LinkHandler(Registers *regs){
    printf("Link handler, for now stubbed\n");
    return 0;
}

uint32_t UnlinkHandler(Registers *regs){
    printf("Unlink handler, for now stubbed\n");
    return 0;
}

uint32_t ExecveHandler(Registers *regs){
    printf("Execve handler, for now stubbed\n");
    return 0;
}

uint32_t LSeekHandler(Registers *regs){
    printf("LSeek handler, for now stubbed\n");
    return 0;
}

uint32_t GetPidHandler(Registers *regs){
    printf("GetPid handler, for now stubbed\n");
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
    return 0;
}

uint32_t FStatHandler(Registers *regs){
    printf("FStat handler, for now stubbed\n");
    return 0;
}

uint32_t SyscallHandler(Registers *regs){
    switch(regs->eax){
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
        case CLOSE:
            return CloseHandler(regs);
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
        case GET_PID:
            return GetPidHandler(regs);
        case KILL:
            return KillHandler(regs);
        case TIMES:
            return TimesHandler(regs);
        case STAT:
            return StatHandler(regs);
        case FSTAT:
            return FStatHandler(regs);
        default:
            printf("Unhandled syscall %d\n", regs->interrupt);
            printf("  eax=%x ebx=%x ecx=%x edx=%x esi=%x edi=%x\n",
                regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
            printf("  esp=%x ebp=%x eip=%x eflags=%x cs=%x ds=%x ss=%x\n",
                regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
            printf("  interrupt=%x errorcode=%x\n", regs->interrupt, regs->error);
            printf("KERNEL PANIC!\n");
            kpanic();
    }
}