/*
 * systhread.h
 *
 *  Created on: 09.08.2011
 *      Author: myaut
 */

#ifndef SYSTHREAD_H_
#define SYSTHREAD_H_

#include <thread.h>

void create_root_thread();
void create_kernel_thread();
void create_idle_thread();

void switch_to_kernel();
void set_kernel_state(thread_state_t state);

#endif /* SYSTHREAD_H_ */
