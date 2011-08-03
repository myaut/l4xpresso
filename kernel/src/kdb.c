/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/kdb.c
Author: myaut

@LICENSE
*/

#include <types.h>
#include <kdb.h>
#include <config.h>
#include <debug.h>
#include <platform/armv7m.h>



typedef void (*kdb_function_t)(void);

struct kdb_t {
	char option;
	char* menuentry;
	kdb_function_t function;
};

extern void kdb_dump_events(void);
extern void kdb_dump_ktable(void);
extern void kdb_show_ktimer(void);
extern void kdb_dump_softirq(void);
extern void kdb_dump_threads(void);

struct kdb_t kdb_functions[] =
{
	{
		.option = 'K',
		.menuentry = "print kernel tables",
		.function = kdb_dump_ktable
	},
	{
		.option = 'e',
		.menuentry = "dump ktimer events",
		.function = kdb_dump_events
	},
	{
		.option = 'n',
		.menuentry = "show timer (now)",
		.function = kdb_show_ktimer
	},
	{
		.option = 's',
		.menuentry = "show softirqs",
		.function = kdb_dump_softirq
	},
	{
		.option = 't',
		.menuentry = "dump threads",
		.function = kdb_dump_threads
	},
	/*Insert KDB functions here*/
};

void kdb_print_menu() {
	int i;

	dbg_printf(DL_KDB, "KDB menu: \n");

	for(i = 0; i < (sizeof(kdb_functions) / sizeof(struct kdb_t)); ++i) {
		dbg_printf(DL_KDB, "%c: %s\n", kdb_functions[i].option, kdb_functions[i].menuentry);
	}
}


int kdb_handler(char c) {
	int i;

	for(i = 0; i <= sizeof(kdb_functions); ++i) {
		if(c == kdb_functions[i].option) {
			kdb_functions[i].function();
			return 0;
		}
	}

	if(c == '?') {
		kdb_print_menu();
		return 0;
	}

	return 1;
}


