/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/softirq.h
Author: myaut

@LICENSE
*/

#ifndef SOFTIRQ_H_
#define SOFTIRQ_H_

#include <platform/armv7m.h>
#include <config.h>
#include <types.h>
#include <thread.h>

typedef enum {
	KTE_SOFTIRQ,		/*Kernel timer event*/
	ASYNC_SOFTIRQ,		/*Asynchronius event*/
	SYSCALL_SOFTIRQ,

#ifdef CONFIG_KDB
	KDB_SOFTIRQ,		/*KDB should have least priority*/
#endif

	NR_SOFTIRQ
} softirq_type_t;

typedef void (*softirq_handler_t)(void );

typedef struct {
	uint32_t 		  schedule;
	softirq_handler_t handler;
} softirq_t;

void softirq_register(softirq_type_t type, softirq_handler_t handler);
void softirq_schedule(softirq_type_t type);
int softirq_isscheduled();
int softirq_execute(void);

/*
 * Context switching is doing on interrupt return
 * We check if nobody schedules actions in kernel (SOFTIRQs)
 * Then do context switch
 *
 * Idea is that on interrupt we'll save all registers under
 * irq_stack_pointer than on return we copy registers to
 * thread's structure or to kernel_ctx
 * */

#define softirq_save_irq() irq_save()
#define softirq_return_irq() 				\
	if(!softirq_isscheduled() && 			\
		thread_isscheduled()) {				\
		uint32_t sp;						\
		sp = thread_ctx_switch(CTX_USER);	\
		irq_return_user(sp);				\
	}										\
	else {									\
		uint32_t sp; 						\
		sp = thread_ctx_switch(CTX_KERNEL); \
		irq_return_kernel(sp);				\
	}

#endif /* SOFTIRQ_H_ */
