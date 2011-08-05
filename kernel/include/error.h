/*
 * error.h
 *
 *  Created on: 05.08.2011
 *      Author: myaut
 */

#ifndef ERROR_H_
#define ERROR_H_

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

#define assert(cond) assert_impl(cond, #cond, __func__)

void set_user_error(enum user_error_t error);
void panic(char* panicmsg);
void assert_impl(int cond, const char* condstr, const char* funcname);

#endif /* ERROR_H_ */
