#include <msp430.h> 
/*
 * main.c
 *
 * Created on: Mar 6, 2016
 * 		Author: Ian Bacus (ianbacus@gmail.com)
 */

#include "constants.h"
#include "periph_initialization.h"
#include "packetGen.h"
#include "packets.h"


void createPackets(void);

volatile engine_t engineA=0, engineB=0;
volatile uint16_t turnouts=0;
volatile uint8_t resetFlag = 0;

/** Delay function. **/
void delay(unsigned int d) {
  volatile uint32_t i;
  for (i = 0; i<d; i++)
  {
  }
}

__attribute__((interrupt(USCIAB0TX_VECTOR))) void SSI_IRQHandler (void)
{
	// Only used to handle transmit interrupts
	static TX_state_t TX_state = Preamble,TX_state_next=LocoA;
	static signed char LocoATimeout = 0, LocoBTimeout = 0;
	static engine_t engineA_sample=0, engineB_sample=0;
	static int8_t turnA_sample=0, turnB_sample=0,turnA_previous_sample=-1,turnB_previous_sample=-1,update=1;
	static uint16_t index = 0;
	FRAME_TYPE data = PREAMBLE_FRAME;
	if(index == 0)
	{
		//Set TX_state here if you want to only send that packet. If you want a preamble before it, \
		set TX_state_next to your desired packet (but don't set TX_state)
		index = 0;
	}
	if (update)
	{
		update = 0;

		if(LocoBTimeout < 0) LocoBTimeout = 0;
		if(LocoATimeout < 0) LocoATimeout = 0;
		engineA_sample = engineA;
		engineB_sample = engineB;
		turnA_sample = (turnouts&0x2)>>1;
		turnB_sample = turnouts&0x1;

	}

	switch(TX_state)
	{
	case Reset:
		data = ResetPacket[index];
		if(++index >= arrayLen(ResetPacket))
		{
			resetFlag = 0;
			LocoATimeout = IDLE_PAD;
			LocoBTimeout = IDLE_PAD;
			TX_state=Preamble;
		}
		break;
	case Idle:
		data = IdlePacket[index];
		if(++index >= arrayLen(IdlePacket))
		{
			index = 0;
			LocoATimeout--;
			LocoBTimeout--;

			TX_state=Preamble;
			TX_state_next = Idle;
			if(LocoATimeout <= 0) TX_state_next = LocoA;
			else if(turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
			else if(LocoBTimeout <= 0) TX_state_next = LocoB;
			else if (turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
		}
		break;
	case Preamble:
		data = PREAMBLE_FRAME;
		if(++index >= PREAMBLE_LEN)
		{
			index=0;
			LocoATimeout--;
			LocoBTimeout--;
			if (resetFlag) TX_state = Reset;
			else TX_state = TX_state_next;

			update = 1;
		}
		break;
	case LocoA:
		data = Engine[0][engineA_sample][index];
		if(++index >= arrayLen(Engine[0][engineA_sample]))
		{
			index=0;
			TX_state=Preamble;
			LocoATimeout = IDLE_PAD;
			LocoBTimeout--;
			if(turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
			else if (LocoBTimeout <= 0) TX_state_next = LocoB;
			else if (turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
			else TX_state_next = Idle;
		}
		break;
	case LocoB:
		data = Engine[1][engineB_sample][index];
		if(++index >= arrayLen(Engine[1][engineB_sample]))
		{
			index=0;
			TX_state=Preamble;
			LocoATimeout--;
			LocoBTimeout = IDLE_PAD;
			if(turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
			else if (LocoATimeout <= 0) TX_state_next = LocoA;
			else if (turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
			else  TX_state_next = Idle;
		}
		break;
	case TurnA:
		data = Turnout[0][1][index];
		if(++index >= arrayLen(Turnout[0][0]))
		{

			turnA_previous_sample = turnA_sample;
			index=0;
			LocoATimeout--;
			LocoBTimeout--;
			TX_state=Preamble;
			if(LocoBTimeout <= 0) TX_state_next = LocoB;
			else if(turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
			else if(LocoATimeout <= 0 ) TX_state_next = LocoA;
			else TX_state_next = Idle;

		}
		break;
	case TurnB:
		data = Turnout[1][1][index];
		if(++index >= arrayLen(Turnout[1][0]))
		{

			turnB_previous_sample = turnB_sample;
			LocoATimeout--;
			LocoBTimeout--;
			index=0;
			TX_state=Preamble;
			if(LocoATimeout <= 0) TX_state_next = LocoA;
			else if(turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
			else if(LocoBTimeout <= 0 ) TX_state_next = LocoB;
			else TX_state_next = Idle;
		}
		break;
	}
	IFG2 = 0;
	UCA0TXBUF = ~data;
}


void main(void)
{
	uint16_t pinData;
	volatile int index = 0,idleCount=0;
	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
	createPackets();  //For testing and development. Packets should be calculated and packed offline instead of on initialization
	//P1DIR &= ~0x7f;
	initDCC_withSSI(1);
	#ifdef REPROGRAM
	index=0;
	while(index != arrayLen(REPROGRAM))
	{
		SSIDataPut(REPROGRAM[index]);
		index++;
	}
	while(idleCount <= 2 )
	{
		index=0;
		while(index != arrayLen(Preamb))
		{
			SSIDataPut(Preamb[index]);
			index++;
		}
		idleCount++;
	}
	while(index != arrayLen(REPROGRAM))
	{
		SSIDataPut(REPROGRAM[index]);
		index++;
	}
#endif
	 __bis_SR_register(GIE);       //enable interrupts
	 while(1)
	 {
		 pinData = P1IN&(~BIT1); //don't read the SSI pin
		 pinData = (pinData&0x1) | (pinData>>1); //just to make it look nice... A/B.Sel/DSSS
//		 engineA = pinData&0xf;
//		 engineB = pinData&0xf;
//		turnouts = 3 & (((0x40&pinData)>>6) | ((0x40&pinData)>>5)) ;

		 turnouts ^=3;
		delay(0x4fff);

	 }
}


void createPackets(void)
{
	uint8_t i,j;
	//Generate packets from strings
	for(i=0;i<2;i++)
	{
		for(j=0;j<arrayLen(Engine_strings[0]);j++)
		{
			updater(Engine_strings[i][j],Engine[i][j],arrayLen(Engine_strings[0][0]),arrayLen(Engine[0][0]));
		}
		for(j=0;j<arrayLen(Turnout_strings[0]);j++)
		{
			updater(Turnout_strings[i][j],Turnout[i][j],arrayLen(Turnout_strings[0][0]),arrayLen(Turnout[0][0]));
		}
	}
	updater(IdlePacket_string,IdlePacket,arrayLen(IdlePacket_string),arrayLen(IdlePacket));
	updater(ResetPacket_string, ResetPacket,arrayLen(ResetPacket_string),arrayLen(ResetPacket));

#ifdef CONSIST
	updater(Consist_strings,ConsistPacket);
#endif
#ifdef REPROGRAM

	updater(FactoryReset_string,FactoryReset,arrayLen(FactoryReset_string),arrayLen(FactoryReset));
	updater(ProgramAddress2_string,ProgramAddress2,arrayLen(ProgramAddress2_string),arrayLen(ProgramAddress2));
	updater(ProgramAddress4_string,ProgramAddress4,arrayLen(ProgramAddress4_string),arrayLen(ProgramAddress4));
#endif
}

