/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /l4xpresso/kernel/src/start.c
Author: myaut

@LICENSE
*/


/*
 * __l4_start initializes microcontroller
 */


#include <platform/armv7m.h>
#include <platform/link.h>
#include <platform/debug_uart.h>
#include <lpc/LPC17xx.h>

#include <types.h>
#include <debug.h>
#include <kdb.h>
#include <ktimer.h>
#include <softirq.h>
#include <syscall.h>
#include <memory.h>
#include <config.h>
#include <l4/utcb.h>

extern dbg_handler_t dbg_handler;


#ifdef CONFIG_KDB
void debug_kdb_handler(void) {
	kdb_handler(dbg_getchar());
}
#endif

extern void root_thread(void);

extern int ipc_deliver(void);

utcb_t 				root_utcb	__KIP;
extern dbg_layer_t dbg_layer;

uint32_t sched_handler(void* data) {
	dbg_puts("S: Scheduler\n");
	schedule();

	return 4096;
}

int main(void) {
	tcb_t* root;

	dbg_uart_init(115200);
	dbg_puts("\n\n---------------------------------------"
			 "\nL4Xpresso hello!\n");

	// dbg_layer = DL_BASIC | DL_KDB | DL_THREAD;

	dbg_layer = ~(DL_KTABLE | DL_KTIMER);

	memory_init();
	syscall_init();
	thread_init_subsys();

	ktimer_event_init();

#	ifdef CONFIG_KDB
	softirq_register(KDB_SOFTIRQ, debug_kdb_handler);
#	endif

	root = thread_init(TID_TO_GLOBALID(THREAD_ROOT), &root_utcb);
	thread_space(root, TID_TO_GLOBALID(THREAD_ROOT), &root_utcb);
	as_map_user(root->as);

	thread_start((void*) &root_stack_end, root_thread, 0, root);

	mpu_enable(MPU_ENABLED);

	ktimer_event_create(4096, sched_handler, NULL);
	ktimer_event_create(4096, ipc_deliver, NULL);

	/* Here is main kernel thread
	 * we will fall here if somebody will
	 * schedule softirq and try to switch context
	 *
	 * If nothing to execute than sleep until next
	 * interrupt arrives (e.g. from kernel timer)*/
	while(1) {
		if(!softirq_execute())
				wait_for_interrupt();
	}
}

void init_zero_seg(uint32_t* dst, uint32_t* dst_end) {
	while(dst < dst_end) {
			*dst++ = 0;
	}
}

void init_copy_seg(uint32_t* src, uint32_t* dst, uint32_t* dst_end) {
	while(dst < dst_end) {
			*dst++ = *src++;
	}
}

void __l4_start() {
	/*Copy data segments*/

	init_copy_seg(&kernel_text_end, &kernel_data_start, &kernel_data_end);		/*DATA (ROM) -> DATA (RAM)*/
	init_copy_seg(&user_text_end, &user_data_start, &user_data_end);			/*USER DATA (ROM) -> USER DATA (RAM)*/

	/*Fill bss with zeroes*/
	init_zero_seg(&kernel_bss_start, &kernel_bss_end);
	init_zero_seg(&kernel_ahb_start, &kernel_ahb_end);
	init_zero_seg(&user_bss_start, &user_bss_end);

	SystemInit();

    main();
}
