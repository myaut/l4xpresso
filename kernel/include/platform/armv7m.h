/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/platform/armv7m.h
Author: myaut

@LICENSE
*/

#ifndef ARMV7M_H_
#define ARMV7M_H_

#include <types.h>

#define __ASM 	__asm
#define	 __INLINE inline

/*
void __set_msp(uint32_t top) __attribute__( ( naked ) );
void __set_msp(uint32_t top)
{
  __ASM volatile ("MSR msp, %0\n\t"
                   "BX  lr     \n\t" : : "r" (top) );
}

void __syscall() {
	__ASM volatile ("SVC #0\n\t");
}*/

/*Bitmap opetations*/
#define BIT_ADDR(addr) (0x22000000 + ((((ptr_t) addr) & 0xFFFFF) << 5))
#define BIT_BITADDR(addr, bitnum) ((uint8_t*) (BIT_ADDR(addr) + (bitnum << 2)))
#define BIT_SHIFT	 2

#endif /* ARMV7M_H_ */
