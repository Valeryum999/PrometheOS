#include <kernel/scheduler.h>

task_struct *current_task_PCB;
task_struct *idle_task;

void initialize_multiprocessing(task_struct *idle_task_copy){
    idle_task = idle_task_copy;
    current_task_PCB = idle_task;
    asm volatile("mov %%esp, %0" : "=r"(idle_task_copy->esp));
    asm volatile("mov %%cr3, %0" : "=r"(idle_task_copy->cr3));
    current_task_PCB->next = current_task_PCB;
}

void schedule(Registers *regs){
    printf("Switching\n");
    switch_to_task(current_task_PCB->next);
}

void add_process_to_schedule(task_struct *next_task){
    if(current_task_PCB == NULL){
        current_task_PCB = next_task;
        current_task_PCB->next = current_task_PCB;
    } else {
        next_task->next = current_task_PCB->next;
        current_task_PCB->next = next_task;
    }
    printf("CR3: %x\n",next_task->cr3);
    printf("Added task ELF: %x\n", next_task->ELFfile);
}

void __attribute__((naked)) task_entry(){
    asm volatile("ret");
}

void exit_process(){
    task_struct *previous_task = current_task_PCB;
    while(previous_task->next != current_task_PCB){
        previous_task = previous_task->next;
    }
    previous_task->next = current_task_PCB->next;
    switch_to_task(current_task_PCB->next);
}