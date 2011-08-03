/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/syscall.c
Author: myaut

@LICENSE
*/

#include <syscall.h>
#include <softirq.h>
#include <thread.h>
#include <debug.h>
#include <platform/armv7m.h>
#include <platform/irq.h>

tcb_t* caller;
uint32_t syscall;

void __svc_handler() {
	caller = thread_current();

	caller->state = T_SVC_BLOCKED;

	softirq_schedule(SYSCALL_SOFTIRQ);
}

IRQ_HANDLER(svc_handler, __svc_handler);

void syscall_init() {
	softirq_register(SYSCALL_SOFTIRQ, syscall_handler);
}

void sys_thread_control(uint32_t* param1, uint32_t* param2) {
	l4_thread_t dest = param1[REG_R0];
	l4_thread_t	space = param1[REG_R1];
	utcb_t* utcb = (void*) param2[1];	/*R4*/

	if(space != L4_NILTHREAD) {
		/*Creation of thread*/
		tcb_t* thr = thread_create(dest, utcb);
		thread_space(thr, space, utcb);
	}
	else {
		/*FIXME: Thread destroy*/
	}
}

void syscall_handler(void) {
	uint32_t svc_num, *svc_param1, *svc_param2;

	svc_param1 = (uint32_t*) caller->ctx.sp;
	svc_num = ((char*) svc_param1[REG_PC])[-2];
	svc_param2 = caller->ctx.regs;

	if(svc_num == SYS_THREAD_CONTROL) {
		/* Simply call thread_create
		 * TODO: checking globalid
		 * TODO: pagers and schedulers*/
		sys_thread_control(svc_param1, svc_param2);
		caller->state = T_RUNNABLE;
	}
#if 0
	else if(svc_num == SYS_IPC) {

		/*TODO: Send-receive phases*/
		tcb_t *to_thr = NULL;
		l4_thread_t to = svc_param1[REG_R0],
				from = svc_param1[REG_R1];

		if(to == L4_NILTHREAD) {
			/*Only receive phases, simply lock myself*/
			caller->state = T_RECV_BLOCKED;
			dbg_printf(DL_IPC, "IPC: %t receiving\n", caller->t_globalid);
		}
		else {
			to_thr =  thread_by_globalid(to);

			if(to_thr->state == T_RECV_BLOCKED) {
				/*To thread is waiting us*/
				to_thr->state = T_RUNNABLE;
				caller->state = T_RUNNABLE;
				dbg_printf(DL_IPC, "IPC: %t to %t\n", caller->t_globalid, to);
			}
			else {
				/*No waiting, block myself*/
				to_thr->t_from = caller->t_globalid;
				caller->state = T_SEND_BLOCKED;

				dbg_printf(DL_IPC, "IPC: %t sending\n", caller->t_globalid);
			}
		}

	}
	else {
		dbg_printf(DL_SYSCALL, "SVC: %d called [%d, %d, %d, %d]\n", svc_num, svc_param1[REG_R0], svc_param1[REG_R1],
					svc_param1[REG_R2], svc_param1[REG_R3]);
		caller->state = T_RUNNABLE;
	}
#endif
}
