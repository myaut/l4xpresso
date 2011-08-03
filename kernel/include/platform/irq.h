/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/platform/irq.h
Author: myaut

@LICENSE
*/

#ifndef IRQ_H_
#define IRQ_H_

#define irq_save()  \
	__ASM volatile("push {r4-r11}"); \
	__ASM volatile("mov r13, %0" : : "r" (irq_stack_pointer));


#define irq_return_kernel(sp)					\
	__ASM volatile("msr msp, %0" : : "r" (sp));	\
	__ASM volatile("mov r0, #0xFFFFFFF1");		\
	__ASM volatile("pop {r4-r11}");				\
	__ASM volatile("mov pc, r0");

#define irq_return_user(sp) 					\
	__ASM volatile("msr psp, %0" : : "r" (sp)); \
	__ASM volatile("mov r0, #0xFFFFFFFD");		\
	__ASM volatile("pop {r4-r11}"); 			\
	__ASM volatile("mov pc, r0");

#define __IRQ __attribute__ ((naked))

extern volatile uint32_t irq_stack_pointer;

#endif /* IRQ_H_ */
