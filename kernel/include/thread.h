/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/include/thread.h
Author: myaut

@LICENSE
*/

#ifndef THREAD_H_
#define THREAD_H_

#include <types.h>
#include <ktable.h>

/*
 * L4 Thread Struct
 * Based on L4 eXperimental Kernel Reference Manual
 * Version X.2 Rev. 7
 *
 * NOTE: Unlike L4, L4Xpresso implements only 8 MRs due to limitations of memory
 *
 */

struct thread_struct {
	l4_thread_t t_globalid;
	l4_thread_t t_localid;

	uint32_t	t_context[16]; 	/*r0-15*/

	struct thread_struct* t_sibling;
	struct thread_struct* t_parent;
	struct thread_struct* t_child;
};

struct thread_ipc {
	uint32_t	mr[8];			/*message registers*/
};

extern volatile struct thread_struct* current;			// Currently on CPU

extern volatile struct thread_struct* root_thread;		// Main task
extern volatile struct thread_struct* sigma0;			// Main memory manager

void thread_init();

#endif /* THREAD_H_ */
