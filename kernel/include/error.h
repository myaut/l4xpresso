/*
 * error.h
 *
 *  Created on: 05.08.2011
 *      Author: myaut
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <platform/link.h>

/**
 * Userspace errors
 */
enum user_error_t {
	UE_NO_PRIVILIGE 	=	1,
	UE_OUT_OF_MEM		=	8,

	/**
	 * ThreadControl errors
	 * see L4 X.2 Reference pp 23 - 24
	 *
	 * Invalid UTCB is not used because in L4Xpresso we have no Utcb Area
	 */
	UE_TC_NOT_AVAILABLE	=	2,
	UE_TC_INVAL_SPACE	=	3,
	UE_TC_INVAL_SCHED	=	4,
	UE_TC_INVAL_UTCB	=	6		/*Not used*/
};

#if 0
#ifndef __PANIC_VAR
#define __PANIC_VAR extern
#endif

__PANIC_VAR uint32_t panic_regs[13];
__PANIC_VAR uint32_t panic_spec_regs[6];
__PANIC_VAR uint32_t panic_pc;
__PANIC_VAR uint32_t panic_lr;

#define EXTRACT_SPEC_REG(reg) \
		__ASM volatile( "mrs r0, " reg "\n" \
						"str r0, [r1]\n" \
						"add r1, #4\n")

#define panic_save_regs() {														\
	__ASM volatile(																\
			"push {r0-r12}\n"	/*Save registers on stack*/						\													\
			"mov r0, %0\n"														\
			"mov r1, #0\n"														\
			"extract:\n"		/*Extract register from stack to panic_regs*/	\
			"pop {r2}\n"														\
			"str r2, [r0]\n"													\
			"sub r0, #4\n"														\
			"add r1, #1\n"														\
			"cmp r1, #14\n"														\
			"bne extract\n"														\
			:																	\
			: "r"(&panic_regs[11]));											\
																				\
	/*Save special regs. Use r1 as a pointer*/									\
	__ASM volatile("mov r1, %0\n" : : "r" (panic_spec_regs));					\
																				\
	EXTRACT_SPEC_REG("PSR");													\
	EXTRACT_SPEC_REG("MSP");													\
	EXTRACT_SPEC_REG("PSP");													\
	EXTRACT_SPEC_REG("BASEPRI");												\
	EXTRACT_SPEC_REG("FAULTMASK");												\
	EXTRACT_SPEC_REG("CONTROL");												\
}
#endif

#define assert(cond) 		assert_impl(cond, #cond, __func__)
#define panic(...) 			panic_impl(__VA_ARGS__)

void set_user_error(enum user_error_t error);
void panic_impl(char* panicfmt, ...);
void panic_dump_stack();
void assert_impl(int cond, const char* condstr, const char* funcname);

#endif /* ERROR_H_ */
