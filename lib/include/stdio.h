#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

bool print(const char* data, size_t length);
int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

#ifdef __cplusplus
}
#endif

#endif
