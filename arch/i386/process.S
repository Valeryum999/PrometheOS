.global switch_to_task
switch_to_task:
    push %ebx
    push %esi
    push %edi
    push %ebp

    mov 12(%esp), %edi           #current PCB
    mov %esp, 4(%edi)

    mov 12(%edi), %esi              #esi = address of the next task's "thread control block" (parameter passed on stack)

    mov 4(%esi), %esp            #Load ESP for next task's stack from the thread's TCB
    mov 8(%esi), %eax         #eax = address of page directory for next task
    mov 16(%esi), %ebx        #ebx = address for the top of the next task's kernel stack
    mov %cr3, %ecx                   #ecx = previous task's virtual address space

    cmp %eax, %ecx                   #Does the virtual address space need to being changed?
    je .doneVAS                   # no, virtual address space is the same, so don't reload it and cause TLB flushes
    mov %eax, %cr3                   # yes, load the next task's virtual address space
.doneVAS:

    pop %ebp
    pop %edi
    pop %esi
    push %ebx

    ret                           #Load next task's EIP from its kernel stack