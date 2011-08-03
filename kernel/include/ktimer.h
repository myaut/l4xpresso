/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/ktimer.h
Author: myaut

@LICENSE
*/

#ifndef KTIMER_H_
#define KTIMER_H_

/* 1 ktimer tick is 4096 CPU cycles*/
#define SYSTICK_CLOCK 		4096
#define KTIMER_MINTICKS		128

void ktimer_handler(void);

/* Returns 0 if successfully handled
 * or number ticks if need to be rescheduled*/
typedef uint32_t (*ktimer_event_handler_t)(void* event);


typedef struct ktimer_event {
	struct ktimer_event*   next;
	ktimer_event_handler_t handler;

	uint32_t delta;
	void* 	 data;
} ktimer_event_t;

int ktimer_event_schedule(uint32_t ticks, ktimer_event_t* kte);
int ktimer_event_create(uint32_t ticks, ktimer_event_handler_t handler, void* data);
void ktimer_event_handler(void);

#endif /* KTIMER_H_ */
