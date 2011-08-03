/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/platform/irq.h
Author: myaut

@LICENSE
*/

#ifndef IRQ_H_
#define IRQ_H_

#include <softirq.h>
#include <thread.h>

/*
 * TODO: current implementation is highly ineffective
 * 1. It is does not support Nested Interrupts (because interrupt nesting
 * will rewrite irq_stack_pointer which is global)
 * 2. We always save all registers even if we won't do context switching.
 * */


/*
 * irq_save()
 *
 * Saves {r4-r11}, msp, psp, EXC_RETURN
 * */
#define irq_save()													\
	__ASM volatile("cpsid i");										\
	__ASM volatile("mov r0, %0" : : "r" (irq_window));				\
	__ASM volatile("stm r0, {r4-r11}");								\
	__ASM volatile("mrs r0, msp");									\
	__ASM volatile("mov %0, r0" : "=r" (irq_stack_pointer) : );		\
	__ASM volatile("mrs r0, psp");									\
	__ASM volatile("mov %0, r0" : "=r" (irq_user_sp) : );			\
	__ASM volatile("mov %0, lr" : "=r" (irq_exc_return) : );

#define irq_return()											\
	__ASM volatile("mov r13, %0" : : "r" (irq_stack_pointer));	\
	__ASM volatile("mov r0, %0" : : "r" (irq_window));			\
	__ASM volatile("ldm r0, {r4-r11}");							\
	__ASM volatile("mov lr, %0" : : "r"(irq_exc_return));		\
	__ASM volatile("cpsie i");									\
	__ASM volatile("bx lr");

#define irq_return_kernel(ctx)						  \
	__ASM volatile("mov r0, %0" : : "r" (ctx->sp));	  \
	__ASM volatile("mov r1, %0" : : "r" (ctx->regs)); \
	__ASM volatile("mov lr, #0xFFFFFFF9");			  \
	__ASM volatile("mov r2, #0x0");					  \
	__ASM volatile("ldm r1, {r4-r11}");			 	  \
	__ASM volatile("msr msp, r0"); 					  \
	__ASM volatile("msr control, r2"); 				  \
	__ASM volatile("cpsie i");						  \
	__ASM volatile("bx lr");

#define irq_return_user(ctx) 						  \
	__ASM volatile("mov r0, %0" : : "r" (ctx->sp));   \
	__ASM volatile("mov r1, %0" : : "r" (ctx->regs)); \
	__ASM volatile("mov lr, #0xFFFFFFFD");			  \
	__ASM volatile("mov r2, #0x3");					  \
	__ASM volatile("ldm r1, {r4-r11}");				  \
	__ASM volatile("msr psp, r0"); 					  \
	__ASM volatile("msr control, r2"); 				  \
	__ASM volatile("cpsie i");						  \
	__ASM volatile("bx lr");

#define __IRQ __attribute__ ((naked))

/*
 * Context switching is doing on interrupt return
 * We check if nobody schedules actions in kernel (SOFTIRQs)
 * Then do context switch
 *
 * Idea is that on interrupt we'll save all registers under
 * irq_stack_pointer than on return we copy registers to
 * thread's structure or to kernel_ctx
 * */


#define IRQ_HANDLER(name, sub) 								\
	void name() __IRQ;										\
	void name() {											\
		irq_save();											\
		sub();												\
		do {												\
			context_t* ctx;									\
			thread_context_t where;							\
			if(!softirq_isscheduled() && 					\
				thread_isscheduled()) {						\
				where = CTX_USER;							\
			}												\
			else {											\
				where = CTX_KERNEL;							\
			}												\
			ctx = thread_ctx_switch(where);					\
			if(ctx != NULL)	{								\
				if(where == CTX_USER) {						\
					irq_return_user(ctx);					\
				} 											\
				else {										\
					irq_return_kernel(ctx);					\
				}											\
			}												\
			irq_return();									\
		} while(0);											\
	}

extern volatile uint32_t irq_stack_pointer;
extern volatile uint32_t irq_exc_return;
extern volatile uint32_t irq_user_sp;
extern volatile uint32_t irq_window[8];

#endif /* IRQ_H_ */
