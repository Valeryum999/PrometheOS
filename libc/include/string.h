#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int itoa(uint32_t num, unsigned char* str, int len, int base);
int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
const char* strchr(const char* str, char chr);
char* strcpy(char *dst, const char *src);
size_t strlen(const char*);
void strrev(unsigned char *str);

#ifdef __cplusplus
}
#endif

#endif
