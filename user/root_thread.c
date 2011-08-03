/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/user/root_thread.c
Author: myaut

@LICENSE
*/

#include <l4/thread.h>

void __dummy_thread()  {
	while(1);
	while(1) {
		__asm volatile("mov r0, 0x1000");
		__asm volatile("svc #0");
	}
}

DECLARE_THREAD(dummy_thread, __dummy_thread);
