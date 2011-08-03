/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/ktimer.h
Author: myaut

@LICENSE
*/

#ifndef KTIMER_H_
#define KTIMER_H_

#include <config.h>

void ktimer_handler(void);

/* Returns 0 if successfully handled
 * or number ticks if need to be rescheduled*/
typedef uint32_t (*ktimer_event_handler_t)(void* data);


typedef struct ktimer_event {
	struct ktimer_event*   next;
	ktimer_event_handler_t handler;

	uint32_t delta;
	void* 	 data;
} ktimer_event_t;

void ktimer_event_init();

int ktimer_event_schedule(uint32_t ticks, ktimer_event_t* kte);
int ktimer_event_create(uint32_t ticks, ktimer_event_handler_t handler, void* data);
void ktimer_event_handler(void);

#endif /* KTIMER_H_ */
