#include <string.h>

char* strcpy(char *dst, const char *src){
    char *destCopy = dst;
    if(dst == NULL)
        return NULL;
    
    if(src == NULL){
        *dst = '\0';
        return dst;
    }

    while(*src){
        *dst = *src;
        ++src;
        ++dst;
    }

    *dst = '\0';
    
    return destCopy;
}