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

	thr = (tcb_t*) ktable_alloc(&thread_table);

	thr->ctx.sp = sp;
	/*Stack allocated, now use fake pc*/
	((uint32_t*) sp)[-REG_PC] = pc;

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
	return current != NULL;
}

void thread_schedule(const thread_tag_t tt, tcb_t* thr) {
	if(tt == THREAD_ROOT)
		thr = root_thread;
	else if(tt == THREAD_SIGMA0)
		thr = sigma0;

	if(current)
		next_current = thr;
	else
		current = thr;
}

/* Switch context
 * */
uint32_t thread_ctx_switch(thread_context_t where) {
	if(current && where == CTX_USER && next_current) {
		/*Somebody rescheduled us, swap contexts*/
		current->ctx.sp = irq_stack_pointer;

		/*Do switch user thread*/
		current = next_current;
		next_current = NULL;

		return current->ctx.sp;
	}
	else if(current && where == CTX_KERNEL) {
		/* Switch to kernel from user space*/
		current->ctx.sp = irq_stack_pointer;

		current = NULL;
		return kernel_ctx.sp;
	}

	return irq_stack_pointer; /*nothing changed, going back*/
}
