.global afterFork
afterFork:
    cli
    push %ebx
    push %esi
    push %edi
    push %ebp
    
    mov (current_task_PCB), %edi           #current PCB
    mov %esp, (%edi)                #move the current esp into the current PCB

    mov 20(%esp), %esi              #esi = address of the next task's "thread control block" 
    mov %esi, (current_task_PCB)    #change current_task_PCB with the next one
    mov 4(%esi), %edi               #change current kernel stack top in the TSS
    push %edi
    call load_TSS
    pop %edi
    mov (%esi), %esp
    mov 8(%esi), %eax
    mov %eax, %cr3                   # yes, load the next task's virtual address space
    xor %eax, %eax  #child process return
    sti
    ret