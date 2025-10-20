#include <kernel/cursor.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end){
    i686_outb(0x3d4, 0xa);
    i686_outb(0x3d5, (i686_inb(0x3d5) & 0xc0) | cursor_start);

    i686_outb(0x3d4, 0xa);
    i686_outb(0x3d5, (i686_inb(0x3d5) & 0xc0) | cursor_end);
}

void disable_cursor(){
    i686_outb(0x3d4, 0x0a);
    i686_outb(0x3d5, 0x20);
}

void update_cursor(int x, int y){
    uint16_t pos = y*VGA_WIDTH + x;

    i686_outb(0x3d4, 0x0f);
    i686_outb(0x3d5, (uint8_t)(pos & 0xff));
    i686_outb(0x3d4, 0x0e);
    i686_outb(0x3d5, (uint8_t)((pos >> 8 )& 0xff));
}