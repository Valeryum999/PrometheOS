#include <kernel/isr.h>
#include <kernel/idt.h>
#include <kernel/gdt.h>
#include <kernel/pager.h>

extern void ISR_InitializeGates();
extern void kpanic();
extern void exit_process();

ISRHandler g_ISRHandlers[256];

static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void PageFaultHandler(Registers *regs){
    uint32_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    printf("PF addr @ %x",cr2);
    printf("  eax=%x ebx=%x ecx=%x edx=%x esi=%x edi=%x\n",
        regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
    printf("  esp=%x ebp=%x eip=%x eflags=%x cs=%x ds=%x ss=%x\n",
        regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
    printf("  interrupt=%x errorcode=%x\n", regs->interrupt, regs->error);
    printf("KERNEL PANIC!\n");
    printf("how the fuck do i debug this\n");
    kpanic();
}

#define EXIT 1
#define WRITE 4

void ExitHandler(Registers *regs){
    printf("Exit handler\n");
    asm volatile("jmp exit_process");
}

void WriteHandler(Registers *regs){
    // printf("Write handler, for now stubbed\n");
    printf("%s",regs->ecx);
}

void SyscallHandler(Registers *regs){
    switch(regs->eax){
        case EXIT:
            ExitHandler(regs);
            break;
        case WRITE:
            WriteHandler(regs);
            break;
        default:
            printf("Unhandled syscall %d\n", regs->interrupt);
            printf("  eax=%x ebx=%x ecx=%x edx=%x esi=%x edi=%x\n",
                regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
            printf("  esp=%x ebp=%x eip=%x eflags=%x cs=%x ds=%x ss=%x\n",
                regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
            printf("  interrupt=%x errorcode=%x\n", regs->interrupt, regs->error);
            printf("KERNEL PANIC!\n");
            kpanic();
    }
    
}

void init_ISR(){
    ISR_InitializeGates();
    for(int i=0; i<256; i++)
        enableIDTGate(i);
    g_ISRHandlers[14] = PageFaultHandler;
    g_ISRHandlers[0x80] = SyscallHandler;
}

void ISR_Handler(Registers* regs){
    if(g_ISRHandlers[regs->interrupt] != NULL)
        g_ISRHandlers[regs->interrupt](regs);
    else if(regs->interrupt >= 32)
        printf("Unhandled interrupt %d\n", regs->interrupt);
    else{
        printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        printf("  eax=%x ebx=%x ecx=%x edx=%x esi=%x edi=%x\n",
            regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        printf("  esp=%x ebp=%x eip=%x eflags=%x cs=%x ds=%x ss=%x\n",
            regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        printf("  interrupt=%x errorcode=%x\n", regs->interrupt, regs->error);
        printf("KERNEL PANIC!\n");
        kpanic();
    }   
}

void ISR_RegisterHandler(int interrupt, ISRHandler handler){
    g_ISRHandlers[interrupt] = handler;
    enableIDTGate(interrupt);
}