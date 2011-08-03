/*
L4Xpresso
Copyright (c) 2011, Sergey Klyaus

File: /leo4-mcu/kernel/src/lpc/flasher/ssp.c
Author: myaut

@LICENSE
*/

#include <lpc/LPC17xx.h>
#include <types.h>

#ifdef CONFIG_FLASHER

LPC_SSP_TypeDef *SSPx = LPC_SSP0;


void ssp_init() {
	LPC_SC->PCONP |= CLKPWR_PCONP_PCSSP0 & CLKPWR_PCONP_BITMASK;
}

void ssp_send_8 (uint8_t b)
{
	SSPx->DR = Data;
}

uint8_t ssp_recv_8() {
	return SSPx->DR & 0xFFFF;
}

int ssp_recv(uint8_t* buffer, size_t length) {
	size_t count = 0;

	// Clear status
	SSPx->ICR = SSP_ICR_BITMASK;

	// Check overrun error
	if (SSPx->RIS & SSP_RIS_ROR)
		return (-1);

	/* Read all data from Fifo */
	while (SSPx->SR & SSP_SR_RNE){
		if(count < length)
			buffer[count++] = ssp_recv_8(SSPx);
	}
}

#endif
