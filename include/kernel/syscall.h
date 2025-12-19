#ifndef _KERNEL_SYSCALL_H
#define _KERNEL_SYSCALL_H

#include <kernel/stat.h>
#include <kernel/utsname.h>
#include <kernel/ioctl.h>
#include <kernel/gdt.h>
#include <kernel/scheduler.h>
#include <kernel/page_frame_allocator.h>
#include <kernel/tty.h>
#include <kernel/errno.h>

//b *0xc00061b5

const char syscall_strings[92][9] = {
    "LOG",
    "EXIT",
    "FORK",
    "READ",
    "WRITE",
    "OPEN",
    "CLOSE",
    "WAIT_PID",
    "",
    "LINK",
    "UNLINK",
    "EXECVE",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "LSEEK",
    "GET_PID",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "KILL",
    "",
    "",
    "",
    "",
    "",
    "TIMES",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "IOCTL",
    "FCNTL",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "MMAP",
    "MUNMAP",
};

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
#define CHDIR 12
#define LSEEK 19
#define GETPID 20
#define GETUID 24
#define KILL    37
#define TIMES 43
#define GETGID 47
#define GETEUID 49
#define GETEGID 50
#define IOCTL 54
#define FCNTL 55
#define MMAP 90
#define MUNMAP 91
#define STAT 106
#define FSTAT 108
#define MPROTECT 125
#define UNAME 122
#define GETPGID 132
#define GETDENTS 141
#define OPENAT 295
#define FSTATAT64 300
#define FACCESAT 307
#define ARCH_PRCTL 384

#endif