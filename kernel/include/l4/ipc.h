/*
 * ipc.h
 *
 *  Created on: 12.11.2012
 *      Author: myaut
 */

#ifndef L4_IPC_H_
#define L4_IPC_H_

#define IPC_TI_MAP_GRANT 0x8
#define IPC_TI_GRANT 	 0x2

#define IPC_MR_COUNT	16

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

#endif /* IPC_H_ */
