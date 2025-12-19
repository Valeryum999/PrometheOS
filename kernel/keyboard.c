#include <kernel/keyboard.h>
#include <kernel/scheduler.h>
#include <fs/fat.h>

/* The different modifier keys we support */
#define MOD_NONE  0
#define MOD_CTRL  (1 << 0)
#define MOD_SHIFT (1 << 1)
#define MOD_ALT   (1 << 2)

/* The modifier keys currently pressed */
static unsigned char mod_keys = 0;

void init_keyboard(){
    i686_IRQ_RegisterHandler(1, keyboard_callback);
}

/* A US keymap, courtesy of Bran's tutorial */
unsigned char kbdmix[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '+', /*'´' */0, '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '<',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '-',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,  '<',
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

unsigned char kbdse_shift[128] = {
    0,  27, '!', '\"', '#', '$' /* shift+4 */, '%', '&', '/', '(',	/* 9 */
  ')', '=', '?', '`', '\b',	/* Backspace */
  '\t',			/* Tab */

 'Q', 'W', 'E', 'R',   /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', 'A', 'A', '\n', /* Enter key */
    0,          /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'O', /* 39 */
 '\'', '>',   0,        /* Left shift */
 '*', 'Z', 'X', 'C', 'V', 'B', 'N',            /* 49 */
  'M', ';', ':', '_',   0,              /* Right shift */

  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   '>',
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

unsigned char kbdse_alt[128] = {
    0,  27, 0 /*alt+1*/, '@', 0, '$', 0, 0, '{', '[',	/* 9 */
  ']', '}', '\\', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,  '|',
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

extern DISK *disk;
extern task_struct *current_task_PCB;

char stdin_buffer[0x200];
int stdin_position = 0;

extern stack_t stack;

void keyboard_callback(Registers *regs){
	unsigned char scancode = i686_inb(0x60);
    unsigned char parsed_char = 0;
    if(scancode == 0xe0){
        return;
    }

    /*
	 * Check for modifier keycodes. If present, toggle their state (if necessary).
	 */
	switch (scancode) {
		case 0x2a: /* shift down */
		case 0x36: /* right shift down */
			mod_keys |= MOD_SHIFT;
			return;
			break;
		case 0xaa: /* shift up */
		case 0xb6: /* right shift up */
			mod_keys &= ~MOD_SHIFT;
			return;
			break;

		case 0x1d: /* ctrl down */
			mod_keys |= MOD_CTRL;
			return;
			break;
		case 0x9d: /* ctrl up */
			mod_keys &= ~MOD_CTRL;
			return;
			break;

		case 0x38: /* alt down */
			mod_keys |= MOD_ALT;
			return;
			break;
		case 0xb8: /* alt up */
			mod_keys &= ~MOD_ALT;
			return;
			break;

		default:
			break;
	}

    if (mod_keys == MOD_NONE && !(scancode & 0x80)) {
		// No modifiers
		parsed_char = kbdmix[scancode];
	} else if (mod_keys == MOD_SHIFT && !(scancode & 0x80)) {
		// Shift + key
		parsed_char = kbdse_shift[scancode];
	} else if (mod_keys == MOD_ALT && !(scancode & 0x80)) {
		// Alt + key
		parsed_char = kbdse_alt[scancode];
	}
	else if (mod_keys == MOD_CTRL && scancode == 0x20) {
		// Ctrl-D
		parsed_char = 4; // ASCII End of Transmission, good enough
	} else if ( !(scancode & 0x80) ) { // scancode isn't simply a supported key being released
		printf("Not implemented (scancode = 0x%x)\n", scancode);
		return;
	} else if (scancode & 0x80) {
		// Key was released
		return;
	}

    // if(scancode == 0x48) {
    //   // arrow up
    //   terminal_history_up();
    //   return;
	  // } else if(scancode == 0x50) {
    //   // arrow down
    //   terminal_history_down();
    //   return;
	  // } else if(scancode == 0x4b){
    //   // arrow left
    //   terminal_move_cursor_left();
    //   return;
    // } else if(scancode == 0x4d){
    //   // arrow left
    //   terminal_move_cursor_right();
    //   return;
    // }

    if(parsed_char == '<'){
      // print_memory_mappings(current_task_PCB);
      printf("Stack top is at %x\n", stack.top);
      return;
    }

    // TODO better insert, this sucks
    printf("%c",parsed_char);

    //backspace
    if(parsed_char == '\b'){
      stdin_position--;
      return;
    }
    
    stdin_buffer[stdin_position++] = parsed_char;
    if(parsed_char == '\n' || stdin_position == 0x200){
      FAT_Write(disk, current_task_PCB->fd[0], stdin_position, stdin_buffer);
      stdin_position = 0;
    }
}