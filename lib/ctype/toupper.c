#include <ctype.h>

char toupper(char chr){
    return islower(chr) ? (chr - 'a' + 'A') : chr;
}