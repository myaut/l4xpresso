/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/platform/link.h
Author: myaut

@LICENSE
*/

#ifndef LINK_H_
#define LINK_H_

/*
 * Linker address
 * Defines each kernel
 */
#define kernel_text_start	   0x0000000
extern unsigned long kernel_text_end;
extern unsigned long kernel_data_start;
extern unsigned long kernel_data_end;
extern unsigned long kernel_bss_start;
extern unsigned long kernel_bss_end;
extern unsigned long kernel_kip_start;
extern unsigned long kernel_kip_end;
extern void kernel_stack_addr(void);

#define __BSS 			__attribute__ ((section(".bss")))
#define __KIP 			__attribute__ ((section(".kip")))
#define __ISR_VECTOR	__attribute__ ((section(".isr_vector")))
#define __KTABLE		__attribute__ ((section(".ktable")))


#define __BITMAP		__attribute__ ((section(".bitmap")))

#endif /* LINK_H_ */
