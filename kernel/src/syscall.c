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

void syscall_handler(void) {
	uint32_t svc_num, *svc_param1, *svc_param2;

	svc_param1 = (uint32_t*) caller->ctx.sp;
	svc_num = ((char*) svc_param1[REG_PC])[-2];
	svc_param2 = caller->ctx.regs;

#if 0
	if(svc_num == SYS_THREAD_CONTROL) {
		/* Simply call thread_create
		 * TODO: checking globalid
		 * TODO: custom address spaces
		 * TODO: pagers and schedulers*/
		tcb_t* thr = thread_create(svc_param1[REG_R0], NULL);
		svc_param1[REG_R0] = 0;
		caller->state = T_RUNNABLE;
	}
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
