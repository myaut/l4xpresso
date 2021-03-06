/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /l4xpresso/kernel/src/kdb.c
Author: myaut

@LICENSE
*/

#include <types.h>
#include <kdb.h>
#include <config.h>
#include <debug.h>
#include <platform/armv7m.h>

#ifdef CONFIG_KDB

typedef void (*kdb_function_t)(void);

struct kdb_t {
	char option;
	char* name;
	char* menuentry;
	kdb_function_t function;
};

extern void kdb_dump_events(void);
extern void kdb_dump_ktable(void);
extern void kdb_show_ktimer(void);
extern void kdb_dump_softirq(void);
extern void kdb_dump_threads(void);
extern void kdb_dump_mempool(void);
extern void kdb_dump_as(void);

struct kdb_t kdb_functions[] =
{
	{
		.option = 'K',
		.name = "KTABLES",
		.menuentry = "print kernel tables",
		.function = kdb_dump_ktable
	},
	{
		.option = 'e',
		.name = "KTIMER",
		.menuentry = "dump ktimer events",
		.function = kdb_dump_events
	},
	{
		.option = 'n',
		.name = "NOW",
		.menuentry = "show timer (now)",
		.function = kdb_show_ktimer
	},
	{
		.option = 's',
		.name = "SOFTIRQ",
		.menuentry = "show softirqs",
		.function = kdb_dump_softirq
	},
	{
		.option = 't',
		.name = "THREADS",
		.menuentry = "dump threads",
		.function = kdb_dump_threads
	},
	{
		.option = 'm',
		.name = "MEMPOOLS",
		.menuentry = "dump memory pools",
		.function = kdb_dump_mempool
	},
	{
		.option = 'a',
		.name = "AS",
		.menuentry = "dump address spaces",
		.function = kdb_dump_as
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

	dbg_printf(DL_KDB, "\n\n-------KDB------\n");
	for(i = 0; i < (sizeof(kdb_functions) / sizeof(struct kdb_t)); ++i) {
		if(c == kdb_functions[i].option) {
			dbg_printf(DL_KDB, "-------%s------\n", kdb_functions[i].name);
			kdb_functions[i].function();
			goto ok;
		}
	}

	if(c == '?') {
		kdb_print_menu();
		goto ok;
	}

	return 1;

ok:
	dbg_printf(DL_KDB, "----------------\n");
	return 0;
}

/*On panic dumps everything*/
int kdb_dump_error() {
	int i = 0;

	for(i = 0; i < (sizeof(kdb_functions) / sizeof(struct kdb_t)); ++i) {
		dbg_printf(DL_KDB, "-------%s------\n", kdb_functions[i].name);
		kdb_functions[i].function();
	}

	return 0;
}

#endif
