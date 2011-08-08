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


__INLINE void irq_disable(void) {
	__ASM volatile ("cpsid i");
}

__INLINE void irq_enable(void) {
	__ASM volatile ("cpsie i");
}

__INLINE int irq_number() {
	int irqno;

	__ASM volatile ( "mrs r0, ipsr\n"
					  "mov r0, %0" : "=r" (irqno) : : "r0");

	return irqno;
}

/*
 * irq_save()
 *
 * Saves {r4-r11}, msp, psp, EXC_RETURN
 * */
#define irq_save(ctx)												\
	__ASM volatile("cpsid i");										\
	__ASM volatile("mov r0, %0" : : "r" ((ctx)->regs));			\
	__ASM volatile("stm r0, {r4-r11}");							\
	__ASM volatile("cmp lr, #0xFFFFFFF9");							\
	__ASM volatile("ite eq");										\
	__ASM volatile("mrseq r0, msp");								\
	__ASM volatile("mrsne r0, psp");								\
	__ASM volatile("mov %0, r0" : "=r" ((ctx)->sp) : );

#define irq_return(ctx)											\
	__ASM volatile("mov lr, %0" : : "r"((ctx)->ret));			\
	__ASM volatile("mov r0, %0" : : "r"((ctx)->sp));			\
	__ASM volatile("mov r2, %0" : : "r"((ctx)->ctl));			\
	__ASM volatile("cmp lr, #0xFFFFFFF9");						\
	__ASM volatile("ite eq");									\
	__ASM volatile("msreq msp, r0");							\
	__ASM volatile("msrne psp, r0");							\
	__ASM volatile("mov r0, %0" : : "r" ((ctx)->regs));		\
	__ASM volatile("ldm r0, {r4-r11}");						\
	__ASM volatile("msr control, r2"); 				  		\
	__ASM volatile("cpsie i");									\
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
		irq_save(&current->ctx);							\
		sub();												\
		thread_switch();									\
		irq_return(&current->ctx);							\
	}

extern volatile tcb_t* current;

#endif /* IRQ_H_ */
