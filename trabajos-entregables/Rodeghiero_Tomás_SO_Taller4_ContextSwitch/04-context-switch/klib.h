#pragma once

#include "types.h"

#define true  1
#define false 0
#define NULL  ((void *) 0)

void *memset(void *buf, char c, unsigned int n);
void *memcpy(void *dst, const void *src, unsigned int n);
char *strcpy(char *dst, const char *src);
int  strcmp(const char *s1, const char *s2);
void printf(const char *fmt, ...);

// panic macro
#define panic(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)

#define stop()                                                                 \
    do {                                                                       \
        printf("Kernel stopped at %s:%d\n", __FILE__, __LINE__);               \
        while (1) {}                                                           \
    } while (0)
