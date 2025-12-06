#include <kernel/scheduler.h>

task_struct *current_task_PCB;
task_struct *idle_task;
task_struct *wait_queue_head;
Elf32_auxv_t phdr;
Elf32_auxv_t ph_entsize;
Elf32_auxv_t ph_entcount;
Elf32_auxv_t eip;
uint32_t entryPosition = 0;
uint32_t *sp;

extern ELF32_File *dynamicLoader;

void initialize_multiprocessing(task_struct *idle_task_copy){
    idle_task = idle_task_copy;
    current_task_PCB = idle_task;
    wait_queue_head = idle_task;
    asm volatile("mov %%esp, %0" : "=r"(idle_task_copy->esp));
    asm volatile("mov %%cr3, %0" : "=r"(idle_task_copy->cr3));
    current_task_PCB->next = current_task_PCB;
    wait_queue_head->next = wait_queue_head;
}

void schedule(Registers *regs){
    switch_to_task(current_task_PCB->next);
}

void add_process_to_schedule(task_struct *next_task){
    next_task->next = current_task_PCB->next;
    current_task_PCB->next = next_task;
}

void add_process_to_waitqueue(task_struct *next_task){
    next_task->next = wait_queue_head->next;
    wait_queue_head->next = next_task;
}

task_struct* remove_running_process_from_runqueue(){
    task_struct *previous_task = current_task_PCB;
    while(previous_task->next != current_task_PCB){
        previous_task = previous_task->next;
    }
    previous_task->next = current_task_PCB->next;

    return current_task_PCB;
}

task_struct* remove_waiting_process_from_waitqueue(){
    task_struct *previous_task = wait_queue_head;
    while(previous_task->next != wait_queue_head){
        previous_task = previous_task->next;
    }
    previous_task->next = wait_queue_head->next;
    return wait_queue_head;
}

void deactivate_current_running_task(){
    task_struct *current = remove_running_process_from_runqueue();
    add_process_to_waitqueue(current);
}

void wakeup(){
    task_struct *current = remove_waiting_process_from_waitqueue();
    add_process_to_schedule(current);
}

extern void jump_usermode(void *func);

void task_entry(const char **argv, const char **envp){
    // switch to kernel stack
    sp = current_task_PCB->esp - 0x100;

    int argc = 0;
    if(argv){
        while(argv[argc])
            argc++;
    }

    int envc = 0;
    if(envp){
        while(envp[envc]){
            envc++;
        }
    }

    ELFHeader *eh = current_task_PCB->ELFfile->header;
    Elf32_auxv_t auxv[5] = {
        { AT_PHDR,  { 0x8000000 + eh->ProgramHeaderTablePosition } },
        { AT_PHENT, { eh->ProgramHeaderTableEntrySize } },
        { AT_PHNUM, { eh->ProgramHeaderTableEntryCount } },
        { AT_ENTRY, { (uint32_t)current_task_PCB->eip } },
        { AT_NULL,  { 0 } },
    };

    int auxc = sizeof(auxv) / sizeof(auxv[0]);

    // push auxv
    for (int i = auxc - 1; i >= 0; i--) {
        *(--sp) = auxv[i].a_un.a_val;
        *(--sp) = auxv[i].a_type;
    }

    // push envp
    *(--sp) = 0;
    for (int i = envc - 1; i >= 0; i--) {
        *(--sp) = (uint32_t)envp[i];
    }

    // push argv
    *(--sp) = 0;
    for (int i = argc - 1; i >= 0; i--) {
        *(--sp) = (uint32_t)argv[i];
    }

    // push argc
    *(--sp) = argc;

    entryPosition = 0xa00000 + 0x1dae9; //TODO base addr
    asm volatile(
        "mov sp, %esp \n\t"
        "mov entryPosition, %eax \n\t" //PH_INTERP entry point
        "push %eax         \n\t" //jmp to entry point
        "lea jump_usermode, %eax \n\t"
        "push %eax  \n\t"
        "xor %ebx, %ebx   \n\t"
        "xor %eax, %eax   \n\t"
        "ret"
    );

    __builtin_unreachable();
}