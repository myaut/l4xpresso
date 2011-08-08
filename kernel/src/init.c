/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /l4xpresso/src/init.c
Author: myaut

@LICENSE
*/

#include <platform/link.h>
#include <platform/debug_uart.h>

#include <debug.h>
#include <error.h>

#include <syscall.h>
#include <ktimer.h>

extern void __l4_start(void);
extern void memmanage_handler(void);

void nointerrupt() {
	while(1) {
		;
	}
}

void hard_fault_handler() {
	panic("Kernel panic: Hard fault. Restarting\n");
	// __ASM volatile("mov pc, %0" : : "r"(__l4_start));
}

void nmi_handler() {
	panic("Kernel panic: NMI. Restarting\n");
	// __ASM volatile("mov pc, %0" : : "r"(__l4_start));
}

void ext_interrupt() {
	while(1) {
		;
	}
}


/*
 * Declare NVIC table
 */
extern void (* const g_pfnVectors[])(void);
__ISR_VECTOR
void (* const g_pfnVectors[])(void) = {
	// Core Level - CM3
	&init_stack_addr, 					// The initial stack pointer
	__l4_start,							// The reset handler
	nmi_handler,						// The NMI handler
	hard_fault_handler,						// The hard fault handler
	memmanage_handler,						// The MPU fault handler
	nointerrupt,						// The bus fault handler
	nointerrupt,						// The usage fault handler
	0,										// Reserved
	0,										// Reserved
	0,										// Reserved
	0,										// Reserved
	svc_handler,					// SVCall handler
	nointerrupt,						// Debug monitor handler
	0,										// Reserved
	nointerrupt,							// The PendSV handler
	ktimer_handler, 				// The SysTick handler

	// Chip Level - LPC17
	ext_interrupt,							// 16, 0x40 - WDT
	ext_interrupt,						// 17, 0x44 - TIMER0
	ext_interrupt,						// 18, 0x48 - TIMER1
	ext_interrupt,						// 19, 0x4c - TIMER2
	ext_interrupt,						// 20, 0x50 - TIMER3
	ext_interrupt,						// 21, 0x54 - UART0
	ext_interrupt,						// 22, 0x58 - UART1
	ext_interrupt,						// 23, 0x5c - UART2
	dbg_uart_interrupt,						// 24, 0x60 - UART3
	ext_interrupt,						// 25, 0x64 - PWM1
	ext_interrupt,						// 26, 0x68 - I2C0
	ext_interrupt,						// 27, 0x6c - I2C1
	ext_interrupt,						// 28, 0x70 - I2C2
	ext_interrupt,							// 29, 0x74 - SPI
	ext_interrupt,						// 30, 0x78 - SSP0
	ext_interrupt,						// 31, 0x7c - SSP1
	ext_interrupt,						// 32, 0x80 - PLL0 (Main PLL)
	ext_interrupt,							// 33, 0x84 - RTC
	ext_interrupt,						// 34, 0x88 - EINT0
	ext_interrupt,						// 35, 0x8c - EINT1
	ext_interrupt,						// 36, 0x90 - EINT2
	ext_interrupt,						// 37, 0x94 - EINT3
	ext_interrupt,							// 38, 0x98 - ADC
	ext_interrupt,							// 39, 0x9c - BOD
	ext_interrupt,							// 40, 0xA0 - USB
	ext_interrupt,							// 41, 0xa4 - CAN
	ext_interrupt,							// 42, 0xa8 - GP DMA
	ext_interrupt,							// 43, 0xac - I2S
	ext_interrupt,						// 44, 0xb0 - Ethernet
	ext_interrupt,							// 45, 0xb4 - RITINT
	ext_interrupt,						// 46, 0xb8 - Motor Control PWM
	ext_interrupt,							// 47, 0xbc - Quadrature Encoder
	ext_interrupt,						// 48, 0xc0 - PLL1 (USB PLL)
	ext_interrupt,					// 49, 0xc4 - USB Activity interrupt to wakeup
	ext_interrupt, 				// 50, 0xc8 - CAN Activity interrupt to wakeup
};
