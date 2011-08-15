/*
 * systhread.c
 *
 *  Created on: 08.08.2011
 *      Author: myaut
 */

#include <platform/armv7m.h>
#include <platform/link.h>
#include <softirq.h>
#include <thread.h>
#include <memory.h>
#include <sched.h>

/*
 * @file systhread.c
 * @brief Three main system threads: kernel, root and idle
 * */

static tcb_t* idle;
static tcb_t* kernel;
static tcb_t* root;

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
	/* Reserve 24 words (16 for context and 8 extra-words) for IDLE stack*/
	kernel_stack_end_ptr = ((uint32_t) &kernel_stack_addr - 96) & 0xFFFFFFFC;

	kernel = thread_init(TID_TO_GLOBALID(THREAD_KERNEL), NULL);
	thread_init_ctx((void*) kernel_stack_end_ptr, kernel_thread, kernel);

	sched_slot_dispatch(SSI_SOFTIRQ, kernel);
}

void create_idle_thread() {
	idle = thread_init(TID_TO_GLOBALID(THREAD_IDLE), NULL);
	thread_init_ctx(&kernel_stack_addr, idle_thread, idle);

	sched_slot_dispatch(SSI_IDLE, idle);

	/*Switch IDLE thread (it is always runnable)*/
	idle->state = T_RUNNABLE;
	thread_switch(idle);
}

void set_kernel_state(thread_state_t state) {
	kernel->state = state;
}

void kernel_thread() {
	while(1) {
		softirq_execute();
	}
}

void idle_thread() {
	while(1) {
		wait_for_interrupt();
	}
}

