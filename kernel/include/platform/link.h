/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/platform/link.h
Author: myaut

@LICENSE
*/

#ifndef LINK_H_
#define LINK_H_

#include <types.h>

/*
 * Linker address
 * Defines each kernel
 */

extern uint32_t kernel_text_start;
extern uint32_t kernel_text_end;
extern uint32_t kernel_data_start;
extern uint32_t kernel_data_end;
extern uint32_t kernel_bss_start;
extern uint32_t kernel_bss_end;
extern uint32_t kernel_ahb_start;
extern uint32_t kernel_ahb_end;

extern uint32_t user_text_start;
extern uint32_t user_text_end;
extern uint32_t user_data_start;
extern uint32_t user_data_end;
extern uint32_t user_bss_start;
extern uint32_t user_bss_end;

extern void kernel_stack_addr(void);

#define __BSS 			__attribute__ ((section(".bss")))
#define __KIP 			__attribute__ ((section(".kip")))
#define __ISR_VECTOR	__attribute__ ((section(".isr_vector")))
#define __KTABLE		__attribute__ ((section(".ktable")))
#define __BITMAP		__attribute__ ((section(".bitmap")))

#define __USER_TEXT		__attribute__ ((section(".user_text")))
#define __USER_DATA		__attribute__ ((section(".user_data")))
#define __USER_BSS		__attribute__ ((section(".user_bss")))

#endif /* LINK_H_ */
