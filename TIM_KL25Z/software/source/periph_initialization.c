/*
 * periph_initialization.c
 *
 *  Created on: Mar 10, 2016
 *      Author: ian bacus
 */
#include "periph_initialization.h"

//6666666.66667: needs to be divided by 380
void initCommandRX_withGPIO(int8_t interrupts_enabled)
{
	/* Port C: Engine bits and Turnout bits
	 * [7 	6 	5 	4 	3 	2 	1 	-]
	 * [es	d		s		s		s	 tA	 tB
	 */
	uint8_t j;
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK; 
	for (j = 1; j <= 7; j++)   PORTC->PCR[j] |= PORT_PCR_MUX(1);	//engine bits (GPIO)
	PTC->PDDR &= ~0xfe; //inputs
	
}
void initDCC_withSSI(int8_t interrupts_enabled)
{
	//OUTDIV4 is applied as a squared number
	//4*4*5*64 = prescaling of 5120, 16.6666667KHz from 86MHz source
		SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0x3) | SIM_CLKDIV1_OUTDIV4(1);
     SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;      //Turn on clock to D module  
     SIM->SCGC4 |= SIM_SCGC4_SPI0_MASK;       //Enable SPI0 clock  
      
      PORTD->PCR[0] = PORT_PCR_MUX(0x2);    //Set PTD0 to mux 2 [SPI0_PCS0]  
      PORTD->PCR[1] = PORT_PCR_MUX(0x2);    //Set PTD1 to mux 2 [SPI0_SCK]  
      PORTD->PCR[2] = PORT_PCR_MUX(0x2);    //Set PTD2 to mux 2 [SPI0_MOSI]  
      PORTD->PCR[3] = PORT_PCR_MUX(0x2);    //Set PTD3 to mux 2 [SPIO_MISO]  
      
      SPI0->C1 = SPI_C1_MSTR_MASK |SPI_C1_CPHA_MASK|SPI_C1_CPOL_MASK ;   //Set SPI0 to Master & SS pin to auto SS  
        //SPI_C1_CPHA_MASK
      //SPI0->C2 = SPI_C2_MODFEN_MASK;   //Master SS pin acts as slave select output   
        //2, 6
      SPI0->BR = (SPI_BR_SPPR(0x04) | SPI_BR_SPR(0x05));     //Set baud rate prescale divisor to 3 & set baud rate divisor to 64 for baud rate of 15625 hz  
        
      SPI0->C1 |= SPI_C1_SPE_MASK;    //Enable SPI0
  
	if(interrupts_enabled == 1)
	{
		SPI0->C1	|= SPI_C1_SPTIE_MASK;
		NVIC->ISER[0] = 1 << SPI0_IRQn;
	}
	
}


void SSIDataPut(FRAME_TYPE data)
{
	//Blocks until data is finished being pushed
	
	SPI0->D = data;
	
	while(!((SPI0->S)&SPI_S_SPTEF_MASK)){}
	//while((SPI0->S)&SPI_S_SPTEF_MASK){}
}





























