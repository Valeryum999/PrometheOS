#ifndef _KERNEL_CURSOR_H
#define _KERNEL_CURSOR_H

#include <stdint.h>
#include <kernel/tty.h>
#include <kernel/io.h>

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void update_cursor(int x, int y);

#endif