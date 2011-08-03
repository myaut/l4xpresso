/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/l4/thread.h
Author: myaut

@LICENSE
*/

#ifndef L4_THREAD_H_
#define L4_THREAD_H_

#include <platform/link.h>

#define DECLARE_THREAD(name, sub) 					\
	void name(void) __attribute__ ((naked));		\
	void __USER_TEXT name(void) {					\
		__asm volatile (".thumb_func");				\
		sub();										\
		while(1);									\
	}

#endif /* THREAD_H_ */
