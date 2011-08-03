/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/ipc.h
Author: myaut

@LICENSE
*/

#ifndef IPC_H_
#define IPC_H_

#define IPC_TI_MAP_GRANT 0x8
#define IPC_TI_GRANT 	 0x2

typedef union {
	struct {
		uint16_t	label;
		uint32_t	reserved	: 3;		/*Type of operation*/
		uint32_t	prop		: 1;

		/*Number of words*/
		uint32_t	n_typed : 6;
		uint32_t	n_untyped : 6;
	} s;
	uint32_t raw;
} ipc_msg_tag_t;

typedef union {
	struct {
		uint32_t	header : 4;
		uint32_t	dummy  : 28;
	} s;
	struct {
		uint32_t	header : 4;
		uint32_t	base  : 28;
	} map;
	uint32_t raw;
} ipc_typed_item;

void sys_ipc(uint32_t* param1);
uint32_t ipc_deliver(void* data);

#endif /* IPC_H_ */
