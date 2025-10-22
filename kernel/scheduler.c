#include <kernel/scheduler.h>

task_struct *tasks_head;
// task_struct *current_task;
task_struct* idle_task;

void schedule(){
    i686_DisableInterrupts();
    switch_to_task(tasks_head);
    i686_EnableInterrupts();
    tasks_head = tasks_head->next;
}

void add_process_to_schedule(task_struct *next_task){
    if(tasks_head == NULL){
        tasks_head = next_task;
        tasks_head->next = tasks_head;
        printf("task cr3: %x\n",tasks_head->cr3);
    }
    printf("Added task ELF: %x\n", tasks_head->ELFfile);
}