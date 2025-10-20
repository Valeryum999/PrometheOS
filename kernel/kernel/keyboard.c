#include <kernel/keyboard.h>

void keyboard(Registers *regs){
	uint8_t character = i686_inb(0x60);
    if(character == 0x1e){
        printf("a");
    } else if(!(character & 0x80)) {
        printf("char %x\n", character);
    }
}

void init_keyboard(){
    i686_IRQ_RegisterHandler(1, keyboard);
}