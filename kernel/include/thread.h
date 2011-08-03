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
#include <kip.h>
#include <config.h>

#include <l4/utcb.h>

#define L4_NILTHREAD		0
#define L4_ANYLOCALTHREAD	0xFFFFFFC0
#define L4_ANYTHREAD		0xFFFFFFFF

#define GLOBALID_TO_TID(id)	 (id >> 14)
#define TID_TO_GLOBALID(id)	 (id << 14)

#define THREAD_BY_TID(id)	 thread_by_globalid(TID_TO_GLOBALID(id))

typedef enum {
	THREAD_IDLE,
	THREAD_KERNEL,
	THREAD_ROOT,
	THREAD_SYS = 16,							/*Systembase*/
	THREAD_USER	= CONFIG_INTR_THREAD_MAX		/*Userbase*/
} thread_tag_t;

typedef enum {
	T_INACTIVE,
	T_ACTIVE,
	T_RUNNABLE,
	T_SVC_BLOCKED,
	T_RECV_BLOCKED,
	T_SEND_BLOCKED
} thread_state_t;

typedef struct {
	uint32_t	sp;
	uint32_t	ret;
	uint32_t	ctl;
	uint32_t	regs[8];
} context_t;

struct tcb {
	l4_thread_t t_globalid;
	l4_thread_t t_localid;

	thread_state_t state;

	context_t ctx;

	as_t* as;
	struct utcb* utcb;
	kip_t* kip;

	struct tcb* t_sibling;
	struct tcb* t_parent;
	struct tcb* t_child;

};
typedef struct tcb tcb_t;

typedef enum  { CTX_KERNEL, CTX_USER } thread_context_t;

void thread_init();
uint32_t thread_isdispatched();
tcb_t* thread_by_globalid(l4_thread_t globalid);
tcb_t* thread_create(l4_thread_t globalid, struct utcb* utcb);
void thread_start(void* sp, void* pc, uint32_t xpsr, tcb_t *thr);
void thread_dispatch(tcb_t* thr);
void thread_switch();
tcb_t* thread_current();

#endif /* THREAD_H_ */
