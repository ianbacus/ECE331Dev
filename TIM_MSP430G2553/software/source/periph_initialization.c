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

	//Configure baud rate: 17544 baud, SMCLK runs at 16 MHz

	UCA0BR1 = 0x3; //MSByte
	UCA0BR0 = 0x90; //LSByte
	if(interrupts_enabled == 1)
		IE2 |= UCA0TXIE;    // Enable USCI0 TX interrupt: indicates data has been written to shift register, more can be written
}

void init_comparator(void)
{
	//Comparator is used to drive DCC. Input is fed from internal reference and SPI output

    P1DIR |= BIT7;  //  P1.7 is output
    P1DIR &= ~BIT0;
    CACTL2 = P2CA0;  // P1.0 = +comp, input
    CACTL1 = CARSEL + CAREF_2 + CAON;  //  -comp = 0.5*Vcc; comparator on
    P1SEL |= BIT0 + BIT7;  // P1.7 updates based on CAOUT


}
void initCommands_withPORT1(void)
{
	//Port 1:
	// inputs.. 0111.1010 = 0x7a, pin 2 is used for SPI
	P1DIR &= ~(0x7a);
}



void SSIDataPut(FRAME_TYPE data)
{
	//Blocks until data is finished being pushed
	UCA0TXBUF = ~data;
	while(UCA0STAT&UCBUSY){/*spin*/}
}



