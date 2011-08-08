/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /l4xpresso/kernel/src/softirq.c
Author: myaut

@LICENSE
*/

#include <types.h>
#include <platform/irq.h>
#include <platform/microops.h>
#include <softirq.h>
#include <config.h>
#include <debug.h>

static softirq_t softirq[NR_SOFTIRQ];
static atomic_t softirq_scheduled_flag = 0;


void softirq_register(softirq_type_t type, softirq_handler_t handler) {
	softirq[type].handler 	= handler;
	softirq[type].schedule = 0;
}

void softirq_schedule(softirq_type_t type) {
	atomic_set(&(softirq[type].schedule), 1);
	atomic_set(&softirq_scheduled_flag, 1);
}
int softirq_isscheduled() {
	return atomic_get(&softirq_scheduled_flag) == 1;
}

char* softirq_names[NR_SOFTIRQ] = {
		"Kernel timer events",
		"Asynchronous events",
		"System calls",
		"KDB enters",
};

int softirq_execute(void) {
	uint32_t schedule = 0, executed = 0;
	int i;

	for(i = 0; i < NR_SOFTIRQ; ++i) {
		if(atomic_get(&(softirq[i].schedule)) != 0
				&& softirq[i].handler) {
			softirq[i].handler();

			executed = 1;
			atomic_set(&(softirq[i].schedule), 0);

			dbg_printf(DL_SOFTIRQ, "SOFTIRQ: executing %s\n", softirq_names[i]);
		}
	}

	/* Must ensure that no interrupt reschedule its softirq */
	irq_disable();

	for(i = 0; i < NR_SOFTIRQ; ++i)
		schedule |= softirq[i].schedule;

	atomic_set(&softirq_scheduled_flag, schedule);
	irq_enable();

	return executed;
}

#ifdef CONFIG_KDB

void kdb_dump_softirq(void) {
	int i;

	dbg_printf(DL_KDB, "SOFTIRQ %s\n", atomic_get(&softirq_scheduled_flag)? "scheduled" : "not scheduled");

	for(i = 0; i < NR_SOFTIRQ; ++i) {
		dbg_printf(DL_KDB, "%32s %s\n", softirq_names[i],
				atomic_get(&(softirq[i].schedule))? "scheduled" : "not scheduled");
	}
}

#endif
