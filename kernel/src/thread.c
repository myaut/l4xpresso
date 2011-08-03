/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/thread.c
Author: myaut

@LICENSE
*/

#include <thread.h>

DECLARE_KTABLE(struct thread_struct, ktable_thread, 64);

void thread_init() {
	struct thread_struct *ts1, *ts2, *ts3;

	ktable_init(&ktable_thread);

	ts1 = (struct thread_struct*) ktable_alloc(&ktable_thread);
	ts2 = (struct thread_struct*) ktable_alloc(&ktable_thread);
	ktable_free(&ktable_thread, ts1);
	ts3 = (struct thread_struct*) ktable_alloc(&ktable_thread);
}
