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
#include <config.h>

struct kip {
	uint32_t	kernel_id;
	uint32_t	api_version;
	uint32_t	api_flags;
	uint32_t	kern_desc_ptr;

	uint32_t	reserved1[17];

	/*TODO: finish kip*/

};

struct kip kip __KIP = {
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
	dbg_uart_init(38400);
	dbg_puts("\n\n---------------------------------------"
			"\nL4Xpresso hello!\n");

	// dbg_layer = DL_BASIC | DL_KDB | DL_THREAD;

	dbg_layer = 0xFFFF;

	ktimer_event_init();
	ktimer_event_create(65535,  wdt_handler, NULL);
	syscall_init();

#	ifdef CONFIG_KDB
	softirq_register(KDB_SOFTIRQ, debug_kdb_handler);
#	endif

	thread_start(&kernel_kip_start, dummy_thread, thread_create(THREAD_ROOT, 0));

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

void __l4_start() {
	/*Copy data segment*/

	uint32_t *data_src =  &kernel_text_end,
			 *data_dst =  &kernel_data_start;

	while(data_dst != &kernel_data_end)
		*data_dst++ = *data_src++;

	/*Copy and zero kip*/
	data_dst = &kernel_kip_start;

	while(data_dst != &kernel_kip_end) {
		*data_dst = 0;
		*data_dst++ = *data_src++;
	}

    __asm("    ldr     r0, =kernel_bss_start\n"
          "    ldr     r1, =kernel_bss_end\n"
          "    mov     r2, #0\n"
          "    .thumb_func\n"
          "zero_loop:\n"
          "        cmp     r0, r1\n"
          "        it      lt\n"
          "        strlt   r2, [r0], #4\n"
          "        blt     zero_loop");

	SystemInit();

    __main();
}
