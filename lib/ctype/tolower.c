#include <ctype.h>

char tolower(char chr){
    return islower(chr) ?  chr : (chr - 'A' + 'a');
}