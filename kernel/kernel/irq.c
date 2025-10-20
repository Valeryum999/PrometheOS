#include <kernel/irq.h>
#include <kernel/pic.h>

#define PIC_REMAP_OFFSET 0x20

extern void i686_EnableInterrupts();

IRQHandler g_IRQHandlers[16];

void i686_IRQ_Handler(Registers *regs){
    int irq = regs->interrupt - PIC_REMAP_OFFSET;
    if(g_IRQHandlers[irq] != NULL){
        g_IRQHandlers[irq](regs);
    } else {
        printf("Unhandled IRQ %d\n", irq);
    }
    i686_PIC_SendEndOfInterrupt(irq);
}

void i686_IRQ_Initialize(){
    i686_PIC_Configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET+8);
    for(size_t i=0; i<16; i++){
        ISR_RegisterHandler(PIC_REMAP_OFFSET + i, i686_IRQ_Handler);
    }
    i686_EnableInterrupts();
}
void i686_IRQ_RegisterHandler(int irq, IRQHandler handler){
    g_IRQHandlers[irq] = handler;
}