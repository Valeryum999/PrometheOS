#ifndef _KERNEL_SYSCALL_H
#define _KERNEL_SYSCALL_H

#include <kernel/scheduler.h>
#include <kernel/page_frame_allocator.h>

#define EXIT 1
#define FORK 2
#define READ 3
#define WRITE 4
#define OPEN 5
#define CLOSE 6
#define WAIT_PID 7
#define LINK 9
#define UNLINK 10
#define EXECVE 11
#define LSEEK 19
#define GET_PID 20
#define KILL    37
#define TIMES 43
#define STAT 106
#define FSTAT 108

#endif