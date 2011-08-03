/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/start.c
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

struct kip_t {
	uint32_t	kernel_id;
	uint32_t	api_version;
	uint32_t	api_flags;
	uint32_t	kern_desc_ptr;

	uint32_t	reserved1[17];

	/*TODO: finish kip*/

};

struct kip_t kip = {
	.kernel_id = 0x49544D4F,	/*ITMO*/
	.api_version = 0x8407000,	/*Ver. X.2 Rev. 7*/
	.api_flags	= 0x00000000,	/*Little endian 32-bit*/
};

extern dbg_handler_t dbg_handler;


#ifdef CONFIG_KDB
void debug_kdb_handler(void) {
	kdb_handler(dbg_getchar());
}
#endif

extern void dummy_thread(void);
extern dbg_layer_t dbg_layer;

uint32_t wdt_handler(void* data) {
	dbg_puts("WDT: handler\n");

	return 65535;
}

int main(void) {
	as_t* user_as;

	dbg_uart_init(38400);
	dbg_puts("\n\n---------------------------------------"
			"\nL4Xpresso hello!\n");

	// dbg_layer = DL_BASIC | DL_KDB | DL_THREAD;

	dbg_layer = 0xFFFF;

	memory_init();
	ktimer_event_init();
	ktimer_event_create(65535,  wdt_handler, NULL);
	syscall_init();

#	ifdef CONFIG_KDB
	softirq_register(KDB_SOFTIRQ, debug_kdb_handler);
#	endif

	user_as = create_as(0);
	thread_start(0x10007fc0, dummy_thread, thread_create(THREAD_ROOT, 0));

	/* Here is main kernel thread
	 * we will fall here if somebody will
	 * schedule softirq and call softirq_return_irq
	 * so we will do context switching
	 *
	 * If nothing to execute than sleep until next
	 * interrupt arrives (e.g. from kernel timer)*/
	while(1) {
		if(!softirq_execute())
			if(!schedule())
				wait_for_interrupt();
	}
}

void init_zero_seg(uint32_t* dst, uint32_t* dst_end) {
	while(dst != dst_end) {
			*dst++ = 0;
	}
}

void init_copy_seg(uint32_t* src, uint32_t* dst, uint32_t* dst_end) {
	while(dst != dst_end) {
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
