/*
 * error.c
 *
 *  Created on: 05.08.2011
 *      Author: myaut
 */

#include <error.h>
#include <thread.h>
#include <debug.h>
#include <platform/debug_uart.h>

extern volatile tcb_t* caller;

void set_user_error(enum user_error_t error) {
	if(caller) {
		assert(!caller->utcb);

		caller->utcb->error_code = error;
	}
}

void panic(char* panicmsg) {
	dbg_start_panic();

	irq_disable();
	dbg_puts(panicmsg);

	while(1);
}

void assert_impl(int cond, const char* condstr, const char* funcname) {
	if(!cond) {
		/*Write to buffer*/
		dbg_printf(DL_EMERG, "\nKernel assertion %s @%s\n", condstr, funcname);

		panic("Assertion failed");
	}
}
