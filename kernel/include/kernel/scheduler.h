#ifndef _KERNEL_SCHEDULER_H
#define _KERNEL_SCHEDULER_H

#include <kernel/process.h>

void init_first_task();
void schedule();
extern void switch_to_task(task_struct* next_thread);
void add_process_to_schedule(task_struct *next_task);
extern void i686_EnableInterrupts();
extern void i686_DisableInterrupts();

#endif