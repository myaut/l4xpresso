/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/include/thread.h
Author: myaut

@LICENSE
*/

#ifndef THREAD_H_
#define THREAD_H_

#include <types.h>
#include <ktable.h>
#include <memory.h>

typedef enum {
	THREAD_KERNEL,
	THREAD_IDLE,
	THREAD_ROOT,
	THREAD_SIGMA0,
	THREAD_USER
} thread_tag_t;

typedef enum {
	T_FREE,
	T_RUNNABLE,
	T_SVC_BLOCKED
} thread_state_t;

typedef struct {
	uint32_t	sp;
	uint32_t	regs[8];
} context_t;

struct tcb {
	l4_thread_t t_globalid;
	l4_thread_t t_localid;

	thread_tag_t tag;
	thread_state_t state;

	context_t ctx;
	as_t* as;

	struct tcb* t_sibling;
	struct tcb* t_parent;
	struct tcb* t_child;
};
typedef struct tcb tcb_t;

struct thread_ipc {
	uint32_t	mr[8];			/*message registers*/
};

typedef enum  { CTX_KERNEL, CTX_USER } thread_context_t;

void thread_init();
uint32_t thread_isscheduled();
tcb_t* thread_create(const thread_tag_t tt, l4_thread_t globalid, as_t* as);
void thread_start(void* sp, void* pc, tcb_t *thr);
void thread_schedule(const thread_tag_t tt, tcb_t* thr);
context_t* thread_ctx_switch(thread_context_t where);


#endif /* THREAD_H_ */
