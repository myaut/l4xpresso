/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/ipc.h
Author: myaut

@LICENSE
*/

#ifndef IPC_H_
#define IPC_H_

#include <l4/ipc.h>

void sys_ipc(uint32_t* param1);
uint32_t ipc_deliver(void* data);

#endif /* IPC_H_ */
