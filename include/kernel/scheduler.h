#ifndef _KERNEL_SCHEDULER_H
#define _KERNEL_SCHEDULER_H

#include <kernel/process.h>
#include <kernel/isr.h>

void initialize_multiprocessing(task_struct *idle_task_copy);
void schedule();
extern void switch_to_task(task_struct* next_thread);
void add_process_to_schedule(task_struct *next_task);
void add_process_to_waitqueue(task_struct *next_task);
task_struct* remove_running_process_from_runqueue();
task_struct* remove_waiting_process_from_waitqueue();
void deactivate_current_running_task();
void wakeup();
extern void i686_EnableInterrupts();
extern void i686_DisableInterrupts();

#endif