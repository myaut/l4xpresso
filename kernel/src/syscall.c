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

extern volatile tcb_t* current;
tcb_t* caller;
uint32_t syscall;

void __svc_handler() {
	caller = current;

	current->state = T_SVC_BLOCKED;

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

	dbg_printf(DL_SYSCALL, "SVC: %d called [%d, %d, %d, %d]\n", svc_num, svc_param1[REG_R0], svc_param1[REG_R1],
			svc_param1[REG_R2], svc_param1[REG_R3]);

	caller->state = T_RUNNABLE;
}
