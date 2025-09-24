#include <kernel/isr.h>
#include <kernel/idt.h>
#include <kernel/gdt.h>
#include <stdio.h>

extern void ISR_InitializeGates();

void init_ISR(){
    ISR_InitializeGates();
    for(int i=0; i<256; i++)
        enableIDTGate(i);
}

void __attribute__((cdecl)) ISR_Handler(Registers* regs){
    printf("Interrupt %d\n", regs->interrupt);
}