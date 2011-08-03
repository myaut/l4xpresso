/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/memory.c
Author: myaut

@LICENSE
*/

#include <memory.h>

static mempool_t memmap[] = {
	DECLARE_MEMPOOL_2("KTEXT", kernel_text, 0)
};
