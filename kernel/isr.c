#include <kernel/isr.h>
#include <kernel/idt.h>
#include <kernel/gdt.h>
#include <kernel/pager.h>
#include <fs/fat.h>
#include <kernel/scheduler.h>

extern void ISR_InitializeGates();
extern void kpanic();

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

extern int verbose;

void PageFaultHandler(Registers *regs){
    uint32_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    if(regs->error & PAGE_PRESENT){
        if(regs->error & PAGE_WRITABLE){
            void *page = (void *)(cr2 & ~0xfff);
            //remap the current page to another known location
            remap_page(page, (void *)0xbff00000);
            //map a new physical address to copy the previous contents into
            mmap(page, PAGE_SIZE, PAGE_USER | PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
            //copy the whole page memory contents in the new writable page
            memcpy(page, (void *)0xbff00000, PAGE_SIZE);
            return;
        }
    }
    printf("Page Fault addr @ %x error %x\n",cr2, regs->error);
    printf("  eax=%x ebx=%x ecx=%x edx=%x esi=%x edi=%x\n",
        regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
    printf("  esp=%x kern_esp=%x ebp=%x eip=%x eflags=%x\n",
        regs->esp, regs->kern_esp, regs->ebp, regs->eip, regs->eflags);
    printf("cs=%x ds=%x ss=%x gs=%x\n", regs->cs, regs->ds, regs->ss, regs->gs);
    printf("  interrupt=%x errorcode=%x\n", regs->interrupt, regs->error);
    printf("KERNEL PANIC!\n");
    kpanic();
}

void init_ISR(){
    ISR_InitializeGates();
    for(int i=0; i<256; i++)
        enableIDTGate(i);
    g_ISRHandlers[14] = PageFaultHandler;
    // g_ISRHandlers[0x80] = SyscallHandler;
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
        printf("  esp=%x kern_esp=%x ebp=%x eip=%x eflags=%x\n",
            regs->esp, regs->kern_esp, regs->ebp, regs->eip, regs->eflags);
        printf("cs=%x ds=%x ss=%x gs=%x\n", regs->cs, regs->ds, regs->ss, regs->gs);
        printf("  interrupt=%x errorcode=%x\n", regs->interrupt, regs->error);
        printf("KERNEL PANIC!\n");
        kpanic();
    }   
}

void ISR_RegisterHandler(int interrupt, ISRHandler handler){
    g_ISRHandlers[interrupt] = handler;
    enableIDTGate(interrupt);
}