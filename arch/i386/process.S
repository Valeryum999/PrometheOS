.global switch_to_task
switch_to_task:
    cli
    push %ebx
    push %esi
    push %edi
    push %ebp

    mov (current_task_PCB), %edi     # current PCB
    mov %esp, (%edi)                 # move the current esp into the current PCB
    mov 20(%esp), %esi               # esi = address of the next task's "thread control block" 
    mov %esi, (current_task_PCB)     # change current_task_PCB with the next one
    mov 4(%esi), %edi                # change current kernel stack top in the TSS
    push %edi
    call load_TSS
    pop %edi
    mov 20(%esi), %edi               # load new gs_base
    push %edi
    call change_gs_base
    pop %edi
    mov (%esi), %esp
    mov 8(%esi), %eax                # current virtual address space 
    mov %cr3, %ecx                   # ecx = previous task's virtual address space
    cmp %eax, %ecx                   # Does the virtual address space need to being changed?
    je .doneVAS                      # no, virtual address space is the same, so don't reload it and cause TLB flushes
    mov %eax, %cr3                   # yes, load the next task's virtual address space
    
.doneVAS:
    pop %ebp
    pop %edi
    pop %esi
    pop %ebx
    sti
    
    ret                           #Load next task's EIP from its kernel stack