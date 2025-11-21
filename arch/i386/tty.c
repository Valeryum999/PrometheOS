#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>
#include <kernel/cursor.h>

#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static const size_t VGA_RESOLUTION = VGA_WIDTH * VGA_HEIGHT;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;
static uint16_t* history_buffer = (uint16_t*) 0xB9000;
static size_t history_buffer_start_pos;
static size_t history_buffer_end_pos;
static size_t current_start_history_buffer;
static size_t current_end_history_buffer;
static const size_t HISTORY_BUFFER_SIZE = 0x4000;
static const size_t HISTORY_MASK = HISTORY_BUFFER_SIZE / 2 - 1;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	history_buffer_start_pos = 0;
	history_buffer_end_pos = VGA_RESOLUTION;
	current_start_history_buffer = 0;
	current_end_history_buffer = VGA_RESOLUTION;

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
			history_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	uint16_t entry = vga_entry(c, color);
	terminal_buffer[index] = entry;
	history_buffer[(history_buffer_start_pos + index) & HISTORY_MASK] = entry;
}

void terminal_refresh(){
	int debug = 0;
	for(size_t y=0; y<VGA_HEIGHT; y++){
		for(size_t x=0; x<VGA_WIDTH; x++){
			size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = history_buffer[(history_buffer_start_pos + index) & HISTORY_MASK];
		}
		debug++;
	}
}

void terminal_move_up(){
	history_buffer_start_pos = ((size_t) history_buffer_start_pos + VGA_WIDTH) & HISTORY_MASK;
	history_buffer_end_pos = ((size_t) history_buffer_end_pos + VGA_WIDTH) & HISTORY_MASK;
	
	current_end_history_buffer = ((size_t) current_end_history_buffer + VGA_WIDTH) & HISTORY_MASK;
	if(current_end_history_buffer > current_start_history_buffer
		&& current_end_history_buffer < (current_start_history_buffer + VGA_WIDTH))
		current_start_history_buffer = current_end_history_buffer - current_end_history_buffer % VGA_WIDTH;

	for(size_t i=0; i<VGA_WIDTH; i++){
		const size_t index = history_buffer_start_pos + (VGA_HEIGHT-1) * VGA_WIDTH + i;
		history_buffer[index & HISTORY_MASK] = vga_entry(' ',terminal_color);
	}
	
	terminal_refresh();
}

void terminal_history_up(){
	if(history_buffer_start_pos == current_start_history_buffer){
		//can't go up anymore
		return;
	}
	history_buffer_start_pos = 	(history_buffer_start_pos - VGA_WIDTH) & HISTORY_MASK;
	history_buffer_end_pos   = 	(history_buffer_end_pos - VGA_WIDTH) & HISTORY_MASK;
	terminal_refresh();
}

void terminal_history_down(){
	if(history_buffer_end_pos == current_end_history_buffer){
		//can't go down anymore
		return;
	}
	history_buffer_start_pos = ((size_t) history_buffer_start_pos + VGA_WIDTH) & HISTORY_MASK;
	history_buffer_end_pos = ((size_t) history_buffer_end_pos + VGA_WIDTH) & HISTORY_MASK;
	terminal_refresh();
}

void terminal_move_cursor_left(){
	// can't go further left than 0
	if(terminal_column == 0){
		return;
	}
	terminal_column--;
	// update_cursor(terminal_column, terminal_row);
}

void terminal_move_cursor_right(){
	// can't go further right than VGA_WIDTH - 1
	if(terminal_column == VGA_WIDTH - 1){
		return;
	}
	terminal_column++;
	// update_cursor(terminal_column, terminal_row);
}

void terminal_putchar(char c) {
	unsigned char uc = c;
	i686_outb(0xe9, uc);
	if(uc == '\n'){
        terminal_column = 0;
        if(++terminal_row == VGA_HEIGHT){
			terminal_move_up();
            terminal_row--;
		}
        return;
    }

	if(uc == '\t'){
		terminal_column += 4 - (terminal_column % 4);	
        if (terminal_column >= VGA_WIDTH) {
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT){
				terminal_move_up();
				terminal_row--;
			}
		}
		return;
    }

	terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT){
			terminal_move_up();
			terminal_row--;
		}
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
	// update_cursor(terminal_column, terminal_row);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}
