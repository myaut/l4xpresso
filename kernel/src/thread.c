/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/thread.c
Author: myaut

@LICENSE
*/

#include <thread.h>
#include <ktable.h>
#include <config.h>
#include <debug.h>
#include <platform/irq.h>
#include <platform/armv7m.h>

DECLARE_KTABLE(tcb_t, thread_table, CONFIG_MAX_THREADS);

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
tcb_t* thread_create(const thread_tag_t tt, l4_thread_t globalid, as_t* as) {
	tcb_t *thr, *t;

	thr = (tcb_t*) ktable_alloc(&thread_table);
	thr->tag = tt;

	if(tt == THREAD_ROOT) {
		if(!root_thread) {
			root_thread = thr;
		}
		else {
			return NULL;
		}

		if(!sigma0)
			sigma0 = thr;

		thr->as = as;

		thr->t_globalid = tt << 14;
		thr->t_localid = 0;
	}
	else if(tt == THREAD_SIGMA0 && !sigma0) {
		sigma0 = thr;

		thr->as = as;
		thr->t_globalid = tt << 14;
		thr->t_localid = 0;
	}
	else if(tt == THREAD_USER && !current) {
		t = current;

		while(t->t_sibling);
		t->t_sibling = thr;

		thr->t_globalid = globalid;
		thr->t_localid = t->t_localid + (1 << 6);
		thr->as = current->as;
	}

	thr->state = T_FREE;

	return thr;
}

void thread_start(void* sp, void* pc, tcb_t *thr) {
	/* Reserve 8 words for fake context */
	sp -= 8 * sizeof(uint32_t);
	thr->ctx.sp = sp;
	/* Stack allocated, now use fake pc */
	((uint32_t*) sp)[REG_R0] = 0x0;
	((uint32_t*) sp)[REG_R1] = 0x0;
	((uint32_t*) sp)[REG_R2] = 0x0;
	((uint32_t*) sp)[REG_R3] = 0x0;
	((uint32_t*) sp)[REG_R12] = 0x0;
	((uint32_t*) sp)[REG_LR] = 0xFFFFFFFF;
	((uint32_t*) sp)[REG_PC] = (uint32_t) pc;
	((uint32_t*) sp)[REG_xPSR] = 0x1000000;				/* Thumb bit on */

	thr->state = T_RUNNABLE;
}

uint32_t thread_isscheduled() {
	return (current != NULL) || (next_current != NULL);
}

void thread_schedule(const thread_tag_t tt, tcb_t* thr) {
	if(tt == THREAD_ROOT)
		thr = root_thread;
	else if(tt == THREAD_SIGMA0)
		thr = sigma0;

	if(thr->state == T_RUNNABLE) {
		dbg_printf(DL_THREAD, "TCB: scheduled %p\n", thr);
		next_current = thr;
	}
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
			dbg_printf(DL_THREAD, "TCB: switch %p -> %p\n", current, next_current);
		}
		else {
			save_context(&kernel_ctx, irq_stack_pointer);
			dbg_printf(DL_THREAD, "TCB: switch kernel -> %p\n", next_current);
		}

		/*Do switch user thread*/
		current = next_current;
		next_current = NULL;

		as_setup_mpu(current->as);
		mpu_enable(MPU_ENABLED);

		return &current->ctx;
	}
	else if(current && where == CTX_KERNEL) {
		/* Switch to kernel from user space*/
		save_context(&current->ctx, irq_user_sp);

		dbg_printf(DL_THREAD, "TCB: switch %p -> kernel\n", current);

		mpu_enable(MPU_DISABLED);
		current = NULL;
		return &kernel_ctx;
	}

	return NULL; /*nothing changed, going back*/
}

int schedule() {
	tcb_t* thr;
	int idx;

	if(!thread_isscheduled()) {
		for_each_in_ktable(thr, idx, (&thread_table)) {
			if(thr->state != T_RUNNABLE)
				continue;

			thread_schedule(thr->tag, thr);
			return 1;
		}
	}

	return 0;
}

#ifdef CONFIG_KDB

void kdb_dump_threads() {
	tcb_t* thr;
	int idx;

	char* types[] = {"", "", "ROOT", "SIGMA0", "USER"};
	char* state[] = {"FREE", "RUN", "SVC"};

	dbg_printf(DL_KDB, "%6s %8s %8s %6s\n", "type", "global", "local", "state");

	for_each_in_ktable(thr, idx, (&thread_table)) {
		dbg_printf(DL_KDB, "%6s %t %t %6s\n", types[thr->tag], thr->t_globalid, thr->t_localid, state[thr->state]);
	}
}

#endif
