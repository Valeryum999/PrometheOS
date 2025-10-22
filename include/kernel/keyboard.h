#ifndef _KERNEL_KEYBOARD_H
#define _KERNEL_KEYBOARD_H

#include <kernel/irq.h>
#include <kernel/io.h>

void keyboard(Registers *regs);
void init_keyboard();

#endif