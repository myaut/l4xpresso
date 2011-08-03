/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/syscall.h
Author: myaut

@LICENSE
*/

#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <config.h>

typedef enum {
	SYS_KERNEL_INTERFACE, /*Not used, KIP is mapped*/
	SYS_EXCHANGE_REGISTERS,
	SYS_THREAD_CONTROL,
	SYS_SYSTEM_CLOCK,
	SYS_THREAD_SWITCH,
	SYS_SCHEDULE,
	SYS_IPC,
	SYS_LIPC,
	SYS_UNMAP,
	SYS_SPACE_CONTROL,
	SYS_PROCESSOR_CONTROL,
	SYS_MEMORY_CONTROL,
#ifdef CONFIG_DEBUG
	SYS_DBG_PUTSTRING
#endif
} syscall_t;

void svc_handler(void);
void syscall_init(void);
void syscall_handler(void);

#endif /* SYSCALL_H_ */
