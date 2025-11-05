#ifndef _KERNEL_PIC_H
#define _KERNEL_PIC_H

#include <stdint.h>
#include <kernel/io.h>

void i686_PIC_Configure(uint8_t offsetPic1, uint8_t offsetPic2);
void i686_PIC_Mask(int irq);
void i686_PIC_Unmask(int irq);
void i686_PIC_Disable();
void i686_PIC_SendEndOfInterrupt(int irq);
uint16_t i686_PIC_ReadIRQRequestRegister();
uint16_t i686_PIC_ReadInServiceRegister();

#endif