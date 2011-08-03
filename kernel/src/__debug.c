/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/debug.c
Author: myaut

@LICENSE
*/

#include <lpc/LPC17xx.h>
#include <stdarg.h>
#include <debug.h>

extern void __CR_SEMIHOST () __attribute__ ((weak, alias("dbg_flush")));

static char debug_write_buf[DEBUG_OUTPUT_BUFFER_SIZE];
static uint8_t debug_buf_read_index, debug_buf_write_index;

int dbg_flush()
{
	uint8_t len, written;

	len = debug_buf_read_index <= debug_buf_write_index
			? debug_buf_write_index - debug_buf_read_index
			: DEBUG_OUTPUT_BUFFER_SIZE - debug_buf_read_index;

	if(!len)
		return 0;

	// The following if() disables semihosted writes when there is no hosted debugger
	// Otherwise, the target will halt when the semihost __write is called
	if(ISDEBUGACTIVE())
	    written = __write(0, &debug_write_buf[debug_buf_read_index], len);
	else
        written = 0;

	debug_buf_read_index = (debug_buf_read_index + written) % DEBUG_OUTPUT_BUFFER_SIZE;

	if(written != len)
		return written;
	return dbg_flush() + written;
}

int dbg_putchar(uint8_t c)
{
	uint8_t buffer_has_room = 1;

	if((debug_buf_write_index+1) % DEBUG_OUTPUT_BUFFER_SIZE  == debug_buf_read_index || c == '\n')
	{
            buffer_has_room = _debug_printf_flush();
	}

	if(buffer_has_room)
	{
	    debug_write_buf[debug_buf_write_index] = c;
	    debug_buf_write_index =
	            (debug_buf_write_index + 1) % DEBUG_OUTPUT_BUFFER_SIZE;
	    return 1;
	}
	return 0;
}

uint8_t dbg_getchar()
{
    char chr = 0;

    dbg_flush();

    if(ISDEBUGACTIVE())
       chr = __readc();

    return chr;
}

void dbg_puts(char* str) {
	while(*str) {
		if(*str == '\n')
			dbg_putchar('\r');
		dbg_putchar(*(str++));
	}
}

void dbg_puts_x(char* str, int width, const char pad) {
	while(*str) {
		if(*str == '\n')
			dbg_putchar('\r');
		dbg_putchar(*(str++));
		--width;
	}

	while(width > 0) {
		dbg_putchar(pad);
		--width;
	}
}

#define hexchars(x) (((x) < 10) ? ('0' + (x)) : ('a' + ((x) - 10)))

int dbg_put_hex(const uint32_t val, int width, const char pad)
{
    int i, n = 0;
    int nwidth = 0;

    // Find width of hexnumber
    while ((val >> (4 * nwidth)) && ((unsigned) nwidth <  2 * sizeof (val)))
    	nwidth++;
    if (nwidth == 0)
    	nwidth = 1;

    // May need to increase number of printed characters
    if (width == 0 && width < nwidth)
    	width = nwidth;

    // Print number with padding
	for (i = width - nwidth; i > 0; i--, n++)
		dbg_putchar (pad);
    for (i = 4 * (nwidth - 1); i >= 0; i -= 4, n++)
    	dbg_putchar(hexchars ((val >> i) & 0xF));

    return n;
}

void dbg_put_dec(const uint32_t val, const int width, const char pad) {
	uint32_t divisor;
    int digits;

    /* estimate number of spaces and digits */
    for (divisor = 1, digits = 1; val/divisor >= 10; divisor *= 10, digits++);

    /* print spaces */
    for ( ; digits < width; digits++ )
    	dbg_putchar(pad);

    /* print digits */
    do {
    	dbg_putchar(((val/divisor) % 10) + '0');
    } while (divisor /= 10);
}

void dbg_printf(char* fmt, ...) {
	va_list va;
	int mode = 0;	/*0 for usual char, 1 for specifiers*/
	int width = 0;
	char pad = ' ';
	int size = 16;

	va_start(va, fmt);

	while(*fmt) {
		if(*fmt == '%') {
			mode = 1;
			pad = ' ';
			width = 0;
			size = 32;

			fmt++;
			continue;
		}

		if(!mode) {
			if(*fmt == '\n')
				dbg_putchar('\r');
			dbg_putchar(*fmt);
		}
		else {
			switch(*fmt) {
			case 'c':
				dbg_putchar(va_arg(va, uint32_t));
				mode = 0;
				break;
			case 's':
				dbg_puts_x(va_arg(va, char*), width, pad);
				mode = 0;
				break;
			case 'l':
			case 'L':
				size = 64;
				break;
			case 'd':
			case 'D':
				dbg_put_dec((size == 32)? va_arg(va, uint32_t)
								: va_arg(va, uint64_t), width, pad);
				mode = 0;
				break;
			case 'p':
				size = 32;
				width = 8;
				pad = '0';
			case 'x':
			case 'X':
				dbg_put_hex((size == 32)? va_arg(va, uint32_t)
							: va_arg(va, uint64_t), width, pad);
				mode = 0;
				break;
			case '%':
				dbg_putchar('%');
				mode = 0;
				break;
			case '0':
				if(!width) pad = '0';
				break;
			case ' ':
				pad = ' ';
			}

			if(*fmt >= '0' && *fmt <= '9') {
				width = width * 10 + (*fmt - '0');
			}
		}

		fmt++;
	}
}
