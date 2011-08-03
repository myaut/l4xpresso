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
#define	__INLINE inline

/*Bitmap opetations*/
#define BIT_ADDR(addr) (0x22000000 + ((((ptr_t) addr) & 0xFFFFF) << 5))
#define BIT_BITADDR(addr, bitnum) ((uint8_t*) (BIT_ADDR(addr) + (bitnum << 2)))
#define BIT_SHIFT	 2

/*Wait for interrupt*/
#define wait_for_interrupt() __ASM volatile ("wfi")

enum register_stack_t {
	/*Saved by hardware*/
	REG_R0,
	REG_R1,
	REG_R2,
	REG_R3,
	REG_R12,
	REG_LR,
	REG_PC,
	REG_xPSR
};


#endif /* ARMV7M_H_ */
