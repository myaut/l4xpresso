/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/thread.c
Author: myaut

@LICENSE
*/

#include <thread.h>
#include <ktable.h>
#include <platform/irq.h>
#include <platform/armv7m.h>

DECLARE_KTABLE(tcb_t, thread_table, 32);

volatile tcb_t* current;			// Currently on CPU
volatile tcb_t* next_current;		// Next on CPU


volatile tcb_t* root_thread;		// Main task
volatile tcb_t* sigma0;			// Main memory manager

void thread_init() {
	ktable_init(&thread_table);
}

context_t kernel_ctx;

/**
 * Create thread
 */
tcb_t* thread_create(void* sp, void* pc, const thread_tag_t tt) {
	tcb_t *thr, *t;
	int i;

	thr = (tcb_t*) ktable_alloc(&thread_table);

	/* Reserve 8 bytes for fake context */
	sp -= 8;
	thr->ctx.sp = sp;
	/* Stack allocated, now use fake pc */
	((uint32_t*) sp)[REG_R0] = 0x0;
	((uint32_t*) sp)[REG_R2] = 0x0;
	((uint32_t*) sp)[REG_R3] = 0x0;
	((uint32_t*) sp)[REG_R12] = 0x0;
	((uint32_t*) sp)[REG_LR] = 0xFFFFFFFF;
	((uint32_t*) sp)[REG_PC] = pc;		/* +1 says to Cortex M3 that we use Thumb instructions*/
	((uint32_t*) sp)[REG_xPSR] = 0x0;

	if(tt == THREAD_ROOT) {
		if(!root_thread) {
			root_thread = thr;
		}
		else {
			return NULL;
		}
	}
	else if(tt == THREAD_SIGMA0 && !sigma0) {
		sigma0 = thr;
	}
	else if(!current) {
		t = current;

		while(t->t_sibling);
		t->t_sibling = thr;
	}

	return thr;
}

uint32_t thread_isscheduled() {
	return (current != NULL) || (next_current != NULL);
}

void thread_schedule(const thread_tag_t tt, tcb_t* thr) {
	if(tt == THREAD_ROOT)
		thr = root_thread;
	else if(tt == THREAD_SIGMA0)
		thr = sigma0;

	next_current = thr;
}

void save_context(context_t* ctx, uint32_t sp) {
	int i;

	ctx->sp = sp;
	for(i = 0; i < 8; ++i)
		ctx->regs[i] = irq_window[i];
}

/* Switch context
 * */
context_t* thread_ctx_switch(thread_context_t where) {
	if(next_current && where == CTX_USER) {
		/*Somebody rescheduled us, swap contexts*/
		if(current) {
			save_context(&current->ctx, irq_user_sp);
		}
		else {
			save_context(&kernel_ctx, irq_stack_pointer);
		}

		/*Do switch user thread*/
		current = next_current;
		next_current = NULL;

		return &current->ctx;
	}
	else if(current && where == CTX_KERNEL) {
		/* Switch to kernel from user space*/
		save_context(&current->ctx, irq_user_sp);

		current = NULL;
		return &kernel_ctx;
	}

	return NULL; /*nothing changed, going back*/
}
