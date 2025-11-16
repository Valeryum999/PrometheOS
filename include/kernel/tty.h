#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>

void terminal_initialize();
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_refresh();
void terminal_history_up();
void terminal_history_down();
void terminal_move_cursor_left();
void terminal_move_cursor_right();

#endif
