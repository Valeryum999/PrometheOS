#include <string.h>

int itoa(uint32_t num, unsigned char* str, int len, int base){
    uint32_t sum = num;
    int i = 0;
    int digit;

    if(len == 0) 
        return -1;

    do{
        digit = sum % base;

        if(digit < 0xA)
            str[i++] = '0' + digit;
        else
            str[i++] = 'A' + digit - 0xA;

        sum /= base;
    } while(sum && (i < (len - 1)));

    if(sum && (i == (len - 1)))
        return -1;

    str[i] = '\0';
    strrev(str);

    return 0;
}
