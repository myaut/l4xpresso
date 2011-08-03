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

uint32_t test_handler1(ktimer_event_t* kte) {
	dbg_puts("Hello from Siberia!\n");

	return 0;
}

uint32_t test_handler2(ktimer_event_t* kte) {
	dbg_puts("Hello from Yamayka!\n");

	return 163840;
}

uint32_t test_handler3(ktimer_event_t* kte) {
	dbg_puts("Hello from Omikron 8!\n");

	return 81920;
}

int main(void) {
	dbg_uart_init(38400);
	dbg_puts("\n\n---------------------------------------"
			"\nL4Xpresso hello!\n");

	ktimer_event_init();
	ktimer_event_create(65535,  test_handler1, NULL);
	ktimer_event_create(163840, test_handler2, NULL);
	ktimer_event_create(128000, test_handler3, NULL);

	while(1) {
		kdb_handler(dbg_getchar());
	}
}

void __l4_start() {
	/*Copy data segment*/

	uint8_t *data_src = (uint8_t*) &kernel_text_end,
			*data_dst = (uint8_t*) &kernel_data_start;

	while(data_dst != &kernel_data_end)
		*data_dst++ = *data_src++;

	/*Copy and zero kip*/
	data_dst = &kernel_kip_start;

	while(data_dst != &kernel_kip_end) {
		*data_dst = 0;
		*data_dst++ = *data_src++;
	}

    __asm("   ldr     r0, =kernel_bss_start\n"
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
