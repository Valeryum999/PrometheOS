.global switch_to_task
switch_to_task:
    pop %ebx                   # current return pointer
    
    mov (%esp), %edi           #current PCB
    mov %esp, 4(%edi)

    mov 12(%edi), %esi              #esi = address of the next task's "thread control block" (parameter passed on stack)

    mov (%esi), %eax 
    mov %eax, 116(%esp)        #Load ESP since it's going to be popped from the stack
    mov 8(%esi), %eax         #eax = address of page directory for next task
    mov %cr3, %ecx                   #ecx = previous task's virtual address space
    cmp %eax, %ecx                   #Does the virtual address space need to being changed?
    je .doneVAS                   # no, virtual address space is the same, so don't reload it and cause TLB flushes
    mov %eax, %cr3                   # yes, load the next task's virtual address space
    
.doneVAS:
    mov 16(%esi), %eax        #Load next process'EIP into eax
    mov %eax, 144(%esp)      #Move next process'EIP into its stack
    #mov (%esi), %eax
    #mov %eax, %esp            #Load ESP for next task's stack from the thread's TCB
    push %ebx
    ret                           #Load next task's EIP from its kernel stack