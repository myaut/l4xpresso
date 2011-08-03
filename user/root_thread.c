/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /l4xpresso/user/root_thread.c
Author: myaut

@LICENSE
*/

#include <l4/thread.h>
#include <l4/kip.h>
#include <l4/utcb.h>
#include <types.h>

#define L4_THREAD_NUM(n, b) ((b + n) << 14)

int __USER_TEXT L4_ThreadControl(l4_thread_t dest, l4_thread_t SpaceSpecifier, l4_thread_t scheduler,
		l4_thread_t pager, void* UtcbLocation) 	{
	uint32_t result;

	 __asm  volatile ( "ldr r0, %0\n"
			 			 "ldr r1, %1\n"
			 			 "ldr r4, %2\n"
						 "svc #2\n"
						 "str r0, %[output]\n"
						 : [output] "=m"(result)
						 : "m"(dest), "m"(SpaceSpecifier), "m"(UtcbLocation)
						 : "r0", "r1", "r4");

	 return result;
}

void __USER_TEXT L4_Ipc(l4_thread_t dest, l4_thread_t FromSpecifier, uint32_t timeouts)
{
		__asm  volatile ("mov r0, %0\n"
						 "mov r1, %1\n"
							 "svc #6\n" : : "r"(dest), "r"(FromSpecifier) :
							 "r0", "r1");
}

enum	{PING_THREAD, PONG_THREAD};

static char stack[256][2] __USER_BSS;
static char utcbs[128][2] __USER_BSS;

static l4_thread_t threads[2] __USER_BSS;

void __USER_TEXT __ping_thread(void* kip_ptr, void* utcb_ptr) {
	uint32_t delay;

	while(1) {
		L4_Ipc(threads[1], L4_NILTHREAD, 0);
	}
}

void __USER_TEXT __pong_thread(void* kip_ptr, void* utcb_ptr) {
	uint32_t delay;

	while(1) {
		L4_Ipc(L4_NILTHREAD, L4_NILTHREAD, 0);
	}
}

DECLARE_THREAD(ping_thread, __ping_thread);
DECLARE_THREAD(pong_thread, __pong_thread);


void __USER_TEXT __root_thread(kip_t* kip_ptr, utcb_t* utcb_ptr) {
	l4_thread_t myself = utcb_ptr->t_globalid;

	threads[PING_THREAD] = L4_THREAD_NUM(PING_THREAD, kip_ptr->thread_info.s.user_base);	/*Ping*/
	threads[PONG_THREAD] = L4_THREAD_NUM(PONG_THREAD, kip_ptr->thread_info.s.user_base);	/*Pong*/

	L4_ThreadControl(threads[PING_THREAD], myself, 0, 0, &(utcbs[PING_THREAD]));
	L4_ThreadControl(threads[PONG_THREAD], myself, 0, 0, &(utcbs[PONG_THREAD]));

	// L4_ExchangeRegisters(threads[0], 0, &(ping_stack[256]), ping_thread, 0, 0, 0);
	// L4_ExchangeRegisters(threads[1], 0, &(pong_stack[256]), pong_thread, 0, 0, 0);

	while(1) {
		;
	}
}

DECLARE_THREAD(root_thread, __root_thread);

