#ifndef _KERNEL_KEYBOARD_H
#define _KERNEL_KEYBOARD_H

#include <kernel/irq.h>
#include <kernel/io.h>
#include <kernel/tty.h>

void init_keyboard();
void keyboard_callback(Registers *regs);

#endif