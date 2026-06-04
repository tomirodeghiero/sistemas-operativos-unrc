#pragma once
#include "spinlock.h"

extern spinlock console_lock;

int console_putc(char ch);
void console_puts(const char *s);
