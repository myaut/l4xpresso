/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/softirq.h
Author: myaut

@LICENSE
*/

#ifndef SOFTIRQ_H_
#define SOFTIRQ_H_

#include <platform/armv7m.h>
#include <config.h>
#include <types.h>
#include <thread.h>

typedef enum {
	KTE_SOFTIRQ,		/*Kernel timer event*/
	ASYNC_SOFTIRQ,		/*Asynchronius event*/
	SYSCALL_SOFTIRQ,

#ifdef CONFIG_KDB
	KDB_SOFTIRQ,		/*KDB should have least priority*/
#endif

	NR_SOFTIRQ
} softirq_type_t;

typedef void (*softirq_handler_t)(void );

typedef struct {
	uint32_t 		  schedule;
	softirq_handler_t handler;
} softirq_t;

void softirq_register(softirq_type_t type, softirq_handler_t handler);
void softirq_schedule(softirq_type_t type);
int softirq_execute(void);

#endif /* SOFTIRQ_H_ */
