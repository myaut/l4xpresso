/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/ktable.c
Author: myaut

@LICENSE
*/

#include <platform/armv7m.h>
#include <ktable.h>

#include <kdb.h>

void ktable_init(ktable_t* kt) {
	char *kt_ptr = kt->bitmap,
		 *kt_end = kt->bitmap + kt->num / 8;

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
			return kt->data + (i * kt->size);
		}

		bptr += 1 << BIT_SHIFT;
	}

	return NULL;
}

void  ktable_free(ktable_t* kt, void* element) {
	size_t i = ((ptr_t) element - kt->data) / kt->size;

	/*Element is not from this ktable*/
	if(i > kt->num)
		return;

	dbg_printf("KT: %s freed %d [%p]\n", kt->tname, i, element);
	*(BIT_BITADDR(kt->bitmap, i)) = 0x0;
}
