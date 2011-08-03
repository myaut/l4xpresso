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


typedef enum {
	DL_BASIC	= 0x8000,
	DL_KDB 		= 0x4000,
	DL_KTABLE	= 0x0001,
	DL_SOFTIRQ	= 0x0002,
	DL_THREAD	= 0x0004,
	DL_KTIMER	= 0x0008,
	DL_SYSCALL	= 0x0010,
	DL_SCHEDULE	= 0x0020,
	DL_MEMORY	= 0x0040,
	DL_IPC		= 0x0080
} dbg_layer_t;

void dbg_putchar(uint8_t chr);
uint8_t dbg_getchar();
void dbg_puts(char* str);
void dbg_printf(dbg_layer_t layer, char* fmt, ...);

#endif

#endif /* DEBUG_H_ */
