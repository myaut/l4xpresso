/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/ktable.c
Author: myaut

@LICENSE
*/

#include <platform/armv7m.h>
#include <ktable.h>
#include <config.h>
#include <debug.h>

#include <kdb.h>

#ifdef CONFIG_KDB

#define KTABLE_NUM 16
static ktable_t* kdb_ktables[KTABLE_NUM];
static uint8_t	 kdb_ktable_cnt;

void kdb_register_ktable(ktable_t* kt) {
	if(kdb_ktable_cnt < (KTABLE_NUM - 1)) {
		kdb_ktables[kdb_ktable_cnt++] = kt;
	}
}

void kdb_dump_ktable() {
	int i = 0, j;
	ktable_t* kt;

	for(; i < kdb_ktable_cnt; ++i) {
		kt = kdb_ktables[i];

		dbg_printf("\nKT: %s\nbitmap:%p, data:%p, num: %d size: %d\n",
			kt->tname, kt->bitmap, kt->data, kt->num, kt->size);
		/*Dump bitmap*/
		for(j = 0; j < kt->num; ++j) {
			if(j % 64 == 0)
				dbg_printf("%5d: ", j);

			dbg_putchar((*(BIT_BITADDR(kt->bitmap, j)))? 'X': '-');

			if((j + 1) % 64 == 0)
				dbg_puts("\n");
		}
	}
}


#endif

void ktable_init(ktable_t* kt) {
	uint32_t  *kt_ptr = (uint32_t*) kt->bitmap,
			  *kt_end = (uint32_t*) kt->bitmap + kt->num / 8;

	while(kt_ptr != kt_end)
		*(kt_ptr++) = 0x0;

	kdb_register_ktable(kt);
}

void* ktable_alloc(ktable_t* kt) {
	char* bptr = (char*) BIT_ADDR(kt->bitmap);
	size_t i;

	/*Search for free element*/
	for(i = 0; i != kt->num; ++i) {
		if(*bptr == 0) {
			dbg_printf("KT: %s allocated %d [%p]\n", kt->tname, i,
					kt->data + (i * kt->size));

			*bptr = 1;
			return (void*) kt->data + (i * kt->size);
		}

		bptr += 1 << BIT_SHIFT;
	}

	return NULL;
}

uint32_t ktable_getid(ktable_t* kt, void* element) {
	size_t i = ((ptr_t) element - kt->data) / kt->size;

	/*Element is not from this ktable*/
	if(i > kt->num)
		return -1;

	return i;
}

void ktable_free(ktable_t* kt, void* element) {
	size_t i = ktable_getid(kt, element);

	if(i != -1) {
		dbg_printf("KT: %s free %d [%p]\n", kt->tname, i, element);
		*(BIT_BITADDR(kt->bitmap, i)) = 0x0;
	}
}
