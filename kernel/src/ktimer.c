/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/ktimer.c
Author: myaut

@LICENSE
*/

#include <lpc/LPC17xx.h>

#include <debug.h>
#include <ktimer.h>
#include <ktable.h>
#include <softirq.h>
#include <platform/armv7m.h>
#include <platform/microops.h>
#include <platform/irq.h>
#include <config.h>

uint64_t ktimer_now;

DECLARE_KTABLE(ktimer_event_t, ktimer_event_table, CONFIG_MAX_KT_EVENTS);

/*Next chain of events which will be executed*/
ktimer_event_t* event_queue = NULL;

//------------------------------------------------

/* Simple ktimer implementation
 *
 * */

static uint32_t ktimer_enabled = 0;
static uint32_t ktimer_delta =   0;
static long long ktimer_time =   0;

void ktimer_init() {
	SysTick->LOAD	= CONFIG_KTIMER_HEARTBEAT - 12;
	SysTick->VAL	= 0;
	SysTick->CTRL 	= 0x7;
}

void ktimer_disable() {
	if(ktimer_enabled) {
		ktimer_enabled = 0;
	}
}

void ktimer_enable(uint32_t delta) {
	if(!ktimer_enabled) {
		ktimer_delta = delta;
		ktimer_time = 0;
		ktimer_enabled = 1;
	}
}

int ktimer_is_enabled() {
	return ktimer_enabled == 1;
}

void ktimer_handler() __IRQ;
void ktimer_handler() {
	softirq_save_irq();

	++ktimer_now;

	if(ktimer_enabled && ktimer_delta > 0) {
		++ktimer_time;
		--ktimer_delta;

		if(ktimer_delta == 0) {
			ktimer_enabled = 0;
			ktimer_time = ktimer_delta = 0;
			softirq_schedule(KTE_SOFTIRQ);
		}
	}

	softirq_return_irq();
}

void kdb_show_ktimer() {
	dbg_printf("Now is %ld\n", ktimer_now);

	if(ktimer_enabled) {
		dbg_printf("Ktimer T=%d D=%d\n", ktimer_time, ktimer_delta);
	}
}

//------------------------------------------------

inline void ktimer_event_recalc(ktimer_event_t* event, uint32_t new_delta) {
	while(event) {
		event->delta -= new_delta;
		event = event->next;
	}
}

int ktimer_event_schedule(uint32_t ticks, ktimer_event_t* kte) {
	long etime = 0, delta = 0;
	ktimer_event_t *event = NULL, *next_event = NULL;

	if(!ticks)
		return -1;
	ticks -= ktimer_time;

	if(event_queue == NULL) {
		/* All other events are already handled,
		 * so simply schedule and enable timer*/
		dbg_printf("KTE: Scheduled dummy event %p on %d\n", kte, ticks);

		kte->delta = ticks;
		event_queue = kte;

		ktimer_enable(ticks);
	}
	else {
		/* etime is total delta for event from now (-ktimer_value())
		 * on each iteration we add delta between events.
		 *
		 * Search for event chain until etime is larger than ticks
		 * e.g ticks = 80
		 *
		 * 0---17------------60----------------60---...
		 *                   ^                 ^
		 *                   |                 (etime + next_event->delta) =
		 *                   |                            = 120 - 17 = 103
		 * 					 etime = 60 - 17 =  43
		 *
		 * kte is between event(60) and event(120),
		 * delta = 80 - 43 =37
		 * insert and recalculate:
		 *
		 * 0---17------------60-------37-------23---...
		 *
		 * */
		next_event = event_queue;

		if(ticks > event_queue->delta) {
			do {
				event = next_event;
				etime += event->delta;
				delta = ticks - etime;
				next_event = event->next;
			} while(next_event &&
					ticks > (etime + next_event->delta));

			dbg_printf("KTE: Scheduled event %p [%p:%p] with D=%d and T=%d\n",
					kte, event, next_event, delta, ticks);

			/*Insert into chain and recalculate*/
			event->next = kte;
		}
		else {
			/* Event should be scheduled before earlier event */
			dbg_printf("KTE: Scheduled early event %p with T=%d\n",
								kte, event, next_event, delta, ticks);

			event_queue = kte;
			delta = ticks;

			/*Reset timer*/
			ktimer_enable(ticks);
		}

		/*Chaining events*/
		if(delta < CONFIG_KTIMER_MINTICKS)
			delta = 0;

		kte->next = next_event;
		kte->delta = delta;

		ktimer_event_recalc(next_event, delta);
	}

	return 0;
}

int ktimer_event_create(uint32_t ticks, ktimer_event_handler_t handler, void* data) {
	ktimer_event_t* kte;

	if(!handler)
		return -1;

	kte = (ktimer_event_t*) ktable_alloc(&ktimer_event_table);

	/*No available slots*/
	if(kte == NULL)
		return -1;

	kte->next = NULL;
	kte->handler = handler;
	kte->data = data;

	return ktimer_event_schedule(ticks, kte);
}

void ktimer_event_handler(void) {
	ktimer_event_t *event = event_queue,
				   *last_event = NULL,
				   *next_event = NULL;
	uint32_t h_retvalue  = 0;
	uint32_t delta = 0;

	if(!event_queue) {
		/* That is bug if we are here */
		dbg_printf("KTE: OOPS! handler found no events\n");

		ktimer_disable();
		return;
	}

	delta = event->delta;

	/* Search last event in event chain */
	do {
		event = event->next;
	} while(event && event->delta == 0);

	last_event = event;

	/*All rescheduled events will be
	 * scheduled after last event*/
	event = event_queue;
	event_queue = last_event;

	/*Walk Chain*/
	do {
		h_retvalue = event->handler(event);
		next_event = event->next;

		if(h_retvalue != 0x0) {
			dbg_printf("KTE: Handled and rescheduled event %p @%ld\n", event, ktimer_now);
			ktimer_event_schedule(h_retvalue, event);
		}
		else {
			dbg_printf("KTE: Handled event %p @%ld\n", event, ktimer_now);
			ktable_free(&ktimer_event_table, event);
		}

		event = next_event;	/*Guaranteed to be next regardless of
							 re-scheduling*/
	} while(next_event &&
			next_event != last_event);

	if(event_queue) {
		/*Reset ktimer*/
		ktimer_enable(event_queue->delta);
	}
}

void ktimer_event_init(void) {
	ktable_init(&ktimer_event_table);
	ktimer_init();
	softirq_register(KTE_SOFTIRQ, ktimer_event_handler);
}


void kdb_dump_events() {
	ktimer_event_t* event = event_queue;

	dbg_puts("\nktimer events: \n");
	dbg_printf("%8s %12s\n", "EVENT", "DELTA");

	while(event) {
		dbg_printf("%p %12d\n", event, event->delta);

		event = event->next;
	}
}
