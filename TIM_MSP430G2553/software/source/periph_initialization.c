/*
 * periph_initialization.c
 *
 *  Created on: Mar 10, 2016
 *      Author: ian bacus
 */
#include "periph_initialization.h"


void initDCC_withSSI(int8_t interrupts_enabled)
{
	//Output direction (required to configure as MOSI output for SPI mode)
	P1DIR |= BIT2;

	//Secondary peripheral mode enabled: UART tx
	P1SEL |= BIT2;
	P1SEL2 |= BIT2;

	UCA0CTL1 = UCSWRST + (3<<6);			// reset state: USCI Software Reset, enable SMCLK
	UCA0CTL0 |= UCMST + UCMSB + UCSYNC;		// synchronous mode, MSB first, SPI master, 8bit xfer
	UCA0CTL1 &= ~UCSWRST;					// Initialize USCI state machine

	//Configure baud rate: 17544 baud, SMCLK runs at 1.2MHz, prescaler  = 68
	UCA0BR1 = 0; //MSByte
	UCA0BR0 = 68; //LSByte

	if(interrupts_enabled == 1)
		IE2 |= UCA0TXIE;    // Enable USCI0 TX interrupt: indicates data has been written to shift register, more can be written
}


void SSIDataPut(FRAME_TYPE data)
{
	//Blocks until data is finished being pushed
	UCA0TXBUF = ~data;
	while(UCA0STAT&UCBUSY){/*spin*/}
}



