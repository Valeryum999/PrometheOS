#include <kernel/scheduler.h>

task_struct *tasks_head;
// task_struct *current_task;
task_struct* idle_task;

void schedule(Registers *regs){
    printf("Switching\n");
    i686_DisableInterrupts();
    switch_to_task(tasks_head);
    i686_EnableInterrupts();
    tasks_head = tasks_head->next;
 
}

void add_process_to_schedule(task_struct *next_task){
    if(tasks_head == NULL){
        tasks_head = next_task;
        tasks_head->next = tasks_head;
    } else {
        next_task->next = tasks_head->next;
        tasks_head->next = next_task;
    }
    printf("CR3: %x\n",next_task->cr3);
    printf("Added task ELF: %x\n", next_task->ELFfile);
}