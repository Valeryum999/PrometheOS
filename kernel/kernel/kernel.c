#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/pager.h>

extern uint32_t *page_directory;
extern uint32_t *page_table;

void kernel_main(void) {
	terminal_initialize();
	terminal_writestring("Hello World!\n");
	init_recursive_page_tables();
}