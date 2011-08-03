/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/include/platform/debug_uart.h
Author: myaut

@LICENSE
*/

#ifndef DEBUG_UART_H_
#define DEBUG_UART_H_

#include <types.h>
#include <fifo.h>

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

#define SEND_BUFSIZE    2048
#define RECV_BUFSIZE    32

struct dbg_uart_t {
	uint32_t status;
	uint32_t ready;

	/*Queues for RX and TX*/
	struct fifo_t tx;
	struct fifo_t rx;
};

void dbg_uart_init(uint32_t baudrate);
void dbg_uart_interrupt (void);

typedef int (*dbg_handler_t)(char);
void dbg_panic_puts(uint8_t* str);

#endif /* DEBUG_UART_H_ */
