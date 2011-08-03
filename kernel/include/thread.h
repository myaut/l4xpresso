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

/*
 * L4 Thread Struct
 * Based on L4 eXperimental Kernel Reference Manual
 * Version X.2 Rev. 7
 *
 * NOTE: Unlike L4, L4Xpresso implements only 8 MRs due to limitations of memory
 *
 */

typedef struct {
	uint32_t	sp;
} context_t;

struct tcb {
	l4_thread_t t_globalid;
	l4_thread_t t_localid;

	context_t ctx;

	struct tcb* t_sibling;
	struct tcb* t_parent;
	struct tcb* t_child;
};
typedef struct tcb tcb_t;

typedef enum {
	THREAD_ROOT,
	THREAD_SIGMA0,
	THREAD_USER
} thread_tag_t;

struct thread_ipc {
	uint32_t	mr[8];			/*message registers*/
};

typedef enum  { CTX_KERNEL, CTX_USER } thread_context_t;

void thread_init();
uint32_t thread_isscheduled();
tcb_t* thread_create(void* sp, void* pc, const thread_tag_t tt);
void thread_schedule(const thread_tag_t tt, tcb_t* thr);
uint32_t thread_ctx_switch(thread_context_t where);

#endif /* THREAD_H_ */
