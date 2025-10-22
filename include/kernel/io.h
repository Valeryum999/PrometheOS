#ifndef _KERNEL_IO_H
#define _KERNEL_IO_H

#include <stdint.h>

extern void i686_outb(uint16_t port, uint8_t value);
extern uint8_t i686_inb(uint16_t port);

void i686_iowait();

#endif