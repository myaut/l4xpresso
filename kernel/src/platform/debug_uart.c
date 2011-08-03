/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/platform/debug_uart.c
Author: myaut

@LICENSE
*/

#include <platform/debug_uart.h>

static struct dbg_uart_t dbg_uart;
dbg_handler_t dbg_handler;

void dbg_uart_init(uint32_t baudrate)
{
  uint32_t Fdiv;
  uint32_t pclkdiv, pclk;

  LPC_PINCON->PINSEL0 &= ~0x0000000F;
  LPC_PINCON->PINSEL0 |=  0x0000000A;  /* RxD3 is P0.1 and TxD3 is P0.0 */
  LPC_SC->PCONP |= 1<<4 | 1<<25; //Enable PCUART1
  /* By default, the PCLKSELx value is zero, thus, the PCLK for
	all the peripherals is 1/4 of the SystemFrequency. */
  /* Bit 6~7 is for UART3 */
  pclkdiv = (LPC_SC->PCLKSEL1 >> 18) & 0x03;
  switch ( pclkdiv )
  {
  case 0x00:
  default:
	  pclk = SystemCoreClock/4;
	  break;
  case 0x01:
	  pclk = SystemCoreClock;
	  break;
  case 0x02:
	  pclk = SystemCoreClock/2;
	  break;
  case 0x03:
	  pclk = SystemCoreClock/8;
	  break;
  }
  LPC_UART3->LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
  Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */
  LPC_UART3->DLM = Fdiv / 256;
  LPC_UART3->DLL = Fdiv % 256;
  LPC_UART3->LCR = 0x03;		/* DLAB = 0 */
  LPC_UART3->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

  NVIC_EnableIRQ(UART3_IRQn);

  LPC_UART3->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART3 interrupt */

  dbg_uart.ready = 1;
}

static void dbg_uart_recv() {
	uint8_t chr = LPC_UART3->RBR;

	if(dbg_handler) {
		dbg_handler(chr);
	}
	else {
		/*Put symbol on queue*/
		FIFO_PUT(dbg_uart.rx, chr, RECV_BUFSIZE);
	}
}

void dbg_uart_send(int avail) {
	if(avail) {
		if(dbg_uart.tx.length != 0) {
			FIFO_GET(LPC_UART3->THR, dbg_uart.tx, SEND_BUFSIZE);
			dbg_uart.ready = 0;
		}
		else {
			dbg_uart.ready = 1;
		}
	}
}

void dbg_uart_interrupt (void)
{
  uint8_t IIRValue, LSRValue;
  uint8_t chr = chr;

  IIRValue = LPC_UART3->IIR;

  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if ( IIRValue == IIR_RLS )		/* Receive Line Status */
  {
	LSRValue = LPC_UART3->LSR;
	/* Receive Line Status */
	if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
	{
		/* There are errors or break interrupt */
		/* Read LSR will clear the interrupt */
		dbg_uart.status = LSRValue;
		chr = LPC_UART3->RBR;		/* Dummy read on RX to clear
							interrupt, then bail out */
	  return;
	}
	if ( LSRValue & LSR_RDR )	/* Receive Data Ready */
	{
		/* If no error on RLS, normal ready, save into the data buffer. */
		/* Note: read RBR will clear the interrupt */
		dbg_uart_recv();
	}
  }
  else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
  {
	/* Receive Data Available */
	dbg_uart_recv();
  }
  else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
  {
	/* Character Time-out indicator */
	dbg_uart.status |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
  {
	/* THRE interrupt */
	LSRValue = LPC_UART3->LSR;		/* Check status in the LSR to see if
									valid data in U0THR or not */
	dbg_uart_send(LSRValue & LSR_THRE);
  }
}

void dbg_putchar(uint8_t chr)
{
	if(!dbg_uart.ready) {
		while()
		FIFO_PUT(dbg_uart.tx, chr, SEND_BUFSIZE);
	}
	else {
		LPC_UART3->THR = chr;
		dbg_uart.ready = 0;
	}
}

uint8_t dbg_getchar() {
	uint8_t chr = 0;

	FIFO_GET(chr, dbg_uart.rx, RECV_BUFSIZE);

	return chr;
}
