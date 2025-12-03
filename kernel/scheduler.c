#include <kernel/scheduler.h>

task_struct *current_task_PCB;
task_struct *idle_task;
task_struct *wait_queue_head;
Elf32_auxv_t phdr;
Elf32_auxv_t ph_entsize;
Elf32_auxv_t ph_entcount;
Elf32_auxv_t eip;
uint32_t entryPosition = 0;

extern ELF32_File *dynamicLoader;

const char *env = "LD_SHOW_AUXV=0";

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

void __attribute__((naked)) task_entry(){
    phdr.a_type = AT_PHDR;
    phdr.a_un.a_val = 0x8000000 + current_task_PCB->ELFfile->header->ProgramHeaderTablePosition;
    ph_entsize.a_type = AT_PHENT;
    ph_entsize.a_un.a_val = current_task_PCB->ELFfile->header->ProgramHeaderTableEntrySize;
    ph_entcount.a_type = AT_PHNUM;
    ph_entcount.a_un.a_val = current_task_PCB->ELFfile->header->ProgramHeaderTableEntryCount;
    eip.a_type = AT_ENTRY;
    eip.a_un.a_val = (uint32_t)current_task_PCB->eip;
    entryPosition = 0xa00000 + 0x1dae9; //TODO base addr
    asm volatile(
        "xor %eax, %eax   \n\t"
        "push %eax         \n\t" 
        "push %eax         \n\t" //"end" of AUXV
        "lea phdr, %eax  \n\t"
        "push 4(%eax)          \n\t" // push value
        "push (%eax)        \n\t"    // push AT_PHDR
        "lea ph_entsize, %eax  \n\t"
        "push 4(%eax)          \n\t" // push value
        "push (%eax)        \n\t"    // push AT_PHENT
        "lea ph_entcount, %eax  \n\t"
        "push 4(%eax)          \n\t" // push value
        "push (%eax)        \n\t"    // push AT_PHNUM
        "lea eip, %eax  \n\t"
        "push 4(%eax)          \n\t" // push value
        "push (%eax)        \n\t"    // push AT_ENTRY
        "xor %eax, %eax   \n\t"
        "push %eax         \n\t" //"end" of env
        "mov env, %eax  \n\t"
        "push %eax         \n\t" //env
        "xor %eax, %eax    \n\t"
        "push %eax         \n\t" //"end" of argv
        "push %eax         \n\t" //argc
        
        // "mov current_task_PCB, %ebx  \n\t" //eip
        // "mov 16(%ebx), %eax \n\t" // load eip (first field) into eax
        "mov entryPosition, %eax \n\t" //PH_INTERP entry point
        "push %eax         \n\t" //jmp to entry point
        "lea jump_usermode, %eax \n\t"
        "push %eax  \n\t"
        "xor %ebx, %ebx   \n\t"
        "xor %eax, %eax   \n\t"
        "ret"
    );
}