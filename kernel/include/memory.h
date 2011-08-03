/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/include/memory.h
Author: myaut

@LICENSE
*/

#ifndef MEMORY_H_
#define MEMORY_H_

#include <types.h>
#include <platform/link.h>

/*
 * Process mapping
 * Statically defines table of process segments
 * in global address space (flash + sram)
 */
struct pmap {
	char	 name[8];

	memptr_t text_base;
	memptr_t text_size;
	memptr_t bss_base;
	memptr_t bss_size;
};

const struct pmap pmap[] __BSS = {
	/*NAME		TEXT BASE	SIZE		BSS BASE	SIZE	*/
	{"kernel",	0x00001000,	0x00010000, 0x00011000, 0x00020000}.
	{"test", 	0x00030000, 0x00008000, 0x00038000, 0x00080000}
};

/*
 * Memory areas
 */

#endif /* MEMORY_H_ */
