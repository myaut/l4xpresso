/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/include/debug.h
Author: myaut

@LICENSE
*/

#ifndef DEBUG_H_
#define DEBUG_H_

#include <types.h>

#ifndef DEBUG

#define dbg_flush()
#define dbg_printf(...)
#define dbg_putchar(c)
#define dbg_getchar()
#define dbg_puts(s)

#else

#define DEBUG_OUTPUT_BUFFER_SIZE 64

void dbg_putchar(uint8_t chr);
uint8_t dbg_getchar();
void dbg_puts(char* str);
void dbg_printf(char* fmt, ...);

#endif

#endif /* DEBUG_H_ */
