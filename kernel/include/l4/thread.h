/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/l4/thread.h
Author: myaut

@LICENSE
*/

#ifndef L4_THREAD_H_
#define L4_THREAD_H_

#include <platform/armv7m.h>
#include <platform/link.h>

#define L4_NILTHREAD		0

#define DECLARE_THREAD(name, sub) 					\
	void name(void) __attribute__ ((naked));		\
	void __USER_TEXT name(void) {					\
		register void *kip_ptr asm ("r0");		\
		register void *utcb_ptr asm ("r1");		\
		sub(kip_ptr, utcb_ptr);						\
		while(1);									\
	}

#endif /* THREAD_H_ */
