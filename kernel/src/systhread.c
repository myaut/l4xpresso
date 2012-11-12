/*
 * systhread.c
 *
 *  Created on: 08.08.2011
 *      Author: myaut
 */

#include <platform/armv7m.h>
#include <platform/link.h>
#include <platform/irq.h>
#include <softirq.h>
#include <thread.h>
#include <memory.h>
#include <fpage_impl.h>
#include <sched.h>

/*
 * @file systhread.c
 * @brief Three main system threads: kernel, root and idle
 * */

tcb_t* idle;
tcb_t* kernel;
tcb_t* root;

extern void root_thread(void);
utcb_t root_utcb	__KIP;

uint32_t* kernel_stack_end_ptr = 0;

void kernel_thread();
void idle_thread();

void create_root_thread() {
	root = thread_init(TID_TO_GLOBALID(THREAD_ROOT), &root_utcb);
	thread_space(root, TID_TO_GLOBALID(THREAD_ROOT), &root_utcb);
	as_map_user(root->as);

	thread_init_ctx((void*) &root_stack_end, root_thread, root);

	sched_slot_dispatch(SSI_ROOT_THREAD, root);
	root->state = T_RUNNABLE;
}

void create_kernel_thread() {
	kernel = thread_init(TID_TO_GLOBALID(THREAD_KERNEL), NULL);

	thread_init_kernel_ctx(&kernel_stack_end, kernel);

	/* This will prevent running other threads
	 * than kernel until it will be initialized*/
	sched_slot_dispatch(SSI_SOFTIRQ, kernel);
	kernel->state = T_RUNNABLE;
}

void create_idle_thread() {
	idle = thread_init(TID_TO_GLOBALID(THREAD_IDLE), NULL);
	thread_init_ctx((void*) &idle_stack_end, idle_thread, idle);

	sched_slot_dispatch(SSI_IDLE, idle);
	idle->state = T_RUNNABLE;
}

void switch_to_kernel() __NAKED;
void switch_to_kernel() {
	create_kernel_thread();

	current = kernel;
	init_ctx_switch(&kernel->ctx, kernel_thread);
}

void set_kernel_state(thread_state_t state) {
	kernel->state = state;
}

void kernel_thread() {
	while(1) {
		/* If all softirqs processed, call SVC to
		 * switch context immediately*/
		softirq_execute();
		irq_svc();
	}
}

void idle_thread() {
	while(1) {
		wait_for_interrupt();
	}
}

