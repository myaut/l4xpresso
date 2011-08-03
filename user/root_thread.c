/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/user/root_thread.c
Author: myaut

@LICENSE
*/

#include <l4/thread.h>
#include <l4/kip.h>
#include <l4/utcb.h>
#include <types.h>

#define L4_THREAD_NUM(n) ((4 + n) << 14)

void __USER_TEXT L4_ThreadControl(l4_thread_t dest, l4_thread_t SpaceSpecifier, l4_thread_t scheduler,
		l4_thread_t pager, void* UtcbLocation) 	{
	 __asm  volatile ("mov r0, %0\n"
				 "svc #2\n" : : "r"(dest) : "r0");
}


void __USER_TEXT L4_ExchangeRegisters(l4_thread_t dest, uint32_t control, void* sp,
		void* pc, uint32_t flags, l4_thread_t pager, void* UserDefinedHandle) {
	 __asm  volatile ("mov r0, %0\n"
					  "mov r2, %1\n"
					  "mov r3, %2\n"
					  "mov r4, %3\n"
					 "svc #1\n" : : "r"(dest), "r"(sp), "r"(pc), "r"(flags):
					 "r0", "r2", "r3", "r4");
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

void __USER_TEXT __ping_thread() {
	uint32_t delay;

	while(1) {
		L4_Ipc(threads[1], L4_NILTHREAD, 0);
	}
}

void __USER_TEXT __pong_thread() {
	uint32_t delay;

	while(1) {
		L4_Ipc(L4_NILTHREAD, L4_NILTHREAD, 0);
	}
}

DECLARE_THREAD(ping_thread, __ping_thread);
DECLARE_THREAD(pong_thread, __pong_thread);



void __USER_TEXT __root_thread() {
	//unsigned myself = root_utcb.t_globalid;

	threads[0] = L4_THREAD_NUM(0);	/*Ping*/
	threads[1] = L4_THREAD_NUM(1);	/*Pong*/

	L4_ThreadControl(threads[0], 0, 0, 0, 0);
	L4_ThreadControl(threads[1], 0, 0, 0, 0);

	// L4_ExchangeRegisters(threads[0], 0, &(ping_stack[256]), ping_thread, 0, 0, 0);
	// L4_ExchangeRegisters(threads[1], 0, &(pong_stack[256]), pong_thread, 0, 0, 0);

	while(1) {
		;
	}
}

DECLARE_THREAD(root_thread, __root_thread);

