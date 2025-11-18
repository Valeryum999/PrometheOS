#ifndef _KERNEL_IOCTL_H
#define _KERNEL_IOCTL_H

typedef unsigned int cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

struct termios {
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t c_cc[1]; //to fix
	speed_t ibaud;
	speed_t obaud;
};

struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

#define TCGETS	    0x5401
#define TIOCGWINSZ  0x5413

#endif