#include <kernel/logging.h>
#include <stdio.h>

void debug_print(const char *s){
    printf("[ \x1b[96mDEBUG\x1b[39m ] %s\n", s);
}

void ok_print(const char *s){
    printf("[ \x1b[32mOK\x1b[39m ] %s\n", s);
}

void error_print(const char *s){
    printf("[ \x1b[31mFATAL\x1b[39m ] %s\n", s);
}