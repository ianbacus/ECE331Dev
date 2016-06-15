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
/*
* For this setup Vcc = 3.53 V
*  reference = Vcc/2 = 1.77 V
*        Vin = 1.76 V and higher: green LED on?
*        Vin = 1.70 V and lower: red LED on?
*
*/

#define GREEN    BIT0
#define RED      BIT6
#define SWITCH   BIT3

void init_comparator(void)
{

    P1DIR |= BIT7;  //  P1.7 are outputs

    CACTL2 = P2CA0;  // P1.0 = +comp
    CACTL1 = CARSEL + CAREF_2 + CAON;  //  -comp = 0.5*Vcc; comparator on
    P1SEL |= BIT0 + BIT7;  // P1.7 updates based on CAOUT


}
void initCommands_withPORT1(void)
{
	//0111.1011 = 0x7b, pin 2 is used for SPI
	P1DIR &= ~(0x7b);
	P2DIR &= ~(BIT2);
}



void SSIDataPut(FRAME_TYPE data)
{
	//Blocks until data is finished being pushed
	UCA0TXBUF = ~data;
	while(UCA0STAT&UCBUSY){/*spin*/}
}



