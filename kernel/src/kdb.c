/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/kdb.c
Author: myaut

@LICENSE
*/

#include <types.h>
#include <kdb.h>
#include <ktable.h>
#include <debug.h>
#include <platform/armv7m.h>

#define KTABLE_NUM 16
static ktable_t* kdb_ktables[KTABLE_NUM];
static uint8_t	 kdb_ktable_cnt;

int kdb_handler(char c) {
	switch(c) {
	case 'K':
		/*Dump kernel tables*/
		kdb_dump_ktable();
		return 0;
	case '?':
		kdb_print_menu();
		return 0;
	}

	return 1;
}

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

void kdb_print_menu() {
	dbg_printf(	"L4Xpresso KDB Menu:\n"
				" K - print kernel tables"
				"\n\n");
}
