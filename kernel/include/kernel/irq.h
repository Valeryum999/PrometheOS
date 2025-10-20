#ifndef _KERNEL_IRQ_H
#define _KERNEL_IRQ_H

#include <kernel/isr.h>

typedef void (*IRQHandler)(Registers* regs);

void i686_IRQ_Initialize();
void i686_IRQ_RegisterHandler(int irq, IRQHandler handler);

#endif