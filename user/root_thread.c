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

#define L4_SYSCALL_PARAM(reg, param) 								\
	register uint32_t __ ## param asm(reg) = (uint32_t) param;

volatile int __USER_TEXT L4_ThreadControl(l4_thread_t dest, l4_thread_t SpaceSpecifier, l4_thread_t scheduler,
		l4_thread_t pager, void* UtcbLocation) 	{
	 uint32_t	result;

	 __asm (  "ldr r4, %1\n"
			   "svc #2\n"
			   "str r0, %[output]\n"
				: [output] "=m"(result)
				: "m" (UtcbLocation));

	 return result;
}

volatile int __USER_TEXT L4_Ipc(l4_thread_t to, l4_thread_t from, uint32_t timeouts, uint32_t* high_mrs) {
	uint32_t	result;

	 __asm (  "push {r4-r11}\n"
			   "ldr r12, %1\n"
			   "ldm r12, {r4-r11}\n"
			   "svc #6\n"
			   "pop {r4-r11}\n"
			   "str r0, %[output]\n"
				: [output] "=m"(result)
				: "m"(high_mrs));

	 return result;
}

int __USER_TEXT L4_Start(l4_thread_t who, void* pc, void* sp) {
	uint32_t	msg[8];

	msg[0] = 2;
	msg[1] = pc;
	msg[2] = sp;

	L4_Ipc(who, L4_NILTHREAD, 0, msg);
}

int __USER_TEXT L4_Map(l4_thread_t where, memptr_t base, size_t size) {
	uint32_t	msg[8];

	/*Contains 2 untyped words*/
	msg[0] = 2 << 6;
	msg[1] = base & 0xFFFFFFC0 | 0xB;
	msg[2] = size | 0xB;

	L4_Ipc(where, L4_NILTHREAD, 0, msg);
}

enum	{PING_THREAD, PONG_THREAD};

static l4_thread_t threads[2] __USER_BSS;

void __USER_TEXT __ping_thread(void* kip_ptr, void* utcb_ptr) {
	uint32_t msg[8] = {0};

	while(1) {
		L4_Ipc(threads[PONG_THREAD], L4_NILTHREAD, 0, msg);
	}
}

void __USER_TEXT __pong_thread(void* kip_ptr, void* utcb_ptr) {
	uint32_t msg[8] = {0};

	while(1) {
		L4_Ipc(L4_NILTHREAD, threads[PING_THREAD], 0, msg);
	}
}

DECLARE_THREAD(ping_thread, __ping_thread);
DECLARE_THREAD(pong_thread, __pong_thread);

memptr_t __USER_TEXT get_free_base(kip_t* kip_ptr) {
	kip_mem_desc_t* desc = ((void*) kip_ptr) + kip_ptr->memory_info.s.memory_desc_ptr;
	int n = kip_ptr->memory_info.s.n;
	int i = 0;

	for(i = 0; i < n; ++i) {
		if((desc[i].size & 0x3F) == 4)
			return desc[i].base & 0xFFFFFFC0;
	}

	return 0;
}

#define STACK_SIZE 128

void __USER_TEXT __root_thread(kip_t* kip_ptr, utcb_t* utcb_ptr) {
	l4_thread_t myself = utcb_ptr->t_globalid;
	char* free_mem = get_free_base(kip_ptr);

	uint32_t msg[8] = {0};

	/*Allocate utcbs and stacks in Free memory region*/
	char* utcbs[2] = {free_mem, free_mem + UTCB_SIZE};
	char* stacks[2] = {free_mem + 2*UTCB_SIZE, free_mem + 2*UTCB_SIZE + STACK_SIZE};

	threads[PING_THREAD] = L4_THREAD_NUM(PING_THREAD, kip_ptr->thread_info.s.user_base);	/*Ping*/
	threads[PONG_THREAD] = L4_THREAD_NUM(PONG_THREAD, kip_ptr->thread_info.s.user_base);	/*Pong*/

	L4_ThreadControl(threads[PING_THREAD], myself, 0, myself, utcbs[PING_THREAD]);
	L4_ThreadControl(threads[PONG_THREAD], myself, 0, myself, utcbs[PONG_THREAD]);

	L4_Map(myself, stacks[PING_THREAD], STACK_SIZE);
	L4_Map(myself, stacks[PONG_THREAD], STACK_SIZE);

	L4_Start(threads[PING_THREAD], ping_thread, stacks[PING_THREAD] + STACK_SIZE);
	L4_Start(threads[PONG_THREAD], pong_thread, stacks[PONG_THREAD] + STACK_SIZE);

	while(1) {
		L4_Ipc(L4_NILTHREAD, L4_NILTHREAD, 0, msg);
	}
}

DECLARE_THREAD(root_thread, __root_thread);

