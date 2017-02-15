
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
#include "DCC.h"

void createPackets(void);
volatile engine_t engineA=8, engineB=8;
volatile int8_t turnouts=0;

inline void captureMTSOutputs(void)
{
	engineA_DSSS=(engineA_DSSS>>1)&0x3;
	if((engineA_DSSS&0x3) == 0)
		engineA_sample = 8;
	
	engineB_DSSS&=(engineB_DSSS>>1)&0x3;
	if((engineB_DSSS&0x3) == 0)
		engineB_sample = 8;
		
	turnouts = (PTC->PDOR) >> 16;
}

void SPI0_IRQHandler (void)
{
	// Only used to handle transmit interrupts
	static TX_state_t TX_state = Preamble,TX_state_next=LocoA;
	static Loco_state_t Loco_state = Address;
	static signed char LocoATimeout = 0, LocoBTimeout = 0;
	static engine_t engineA_sample=0, engineB_sample=0;
	static int8_t turnA_sample=0, turnB_sample=0,turnA_previous_sample=-1,turnB_previous_sample=-1,update=1;
	static uint8_t index = 0;
	static uint8_t locoPktCount=0;
	FRAME_TYPE data = PREAMBLE_FRAME;
 
	
	if(index == 0)
	{
		//Set TX_state here if you want to only send that packet. If you want a preamble before it, \
		set TX_state_next to your desired packet (but don't set TX_state)
		turnA_previous_sample = -1;
	}
	if (update)
	{
		update = 0;

		if(LocoBTimeout < 0) LocoBTimeout = 0;
		if(LocoATimeout < 0) LocoATimeout = 0;
		//engineA_sample = engineA;
		//engineB_sample = engineB;
		captureMTSOutputs();
		turnA_sample = (turnouts&0x2)>>1;
		turnB_sample = turnouts&0x1;

	}

	switch(TX_state)
	{
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
			else TX_state_next = TurnA;
			/*
			else if(turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
			else if(LocoBTimeout <= 0) TX_state_next = LocoB;
			else if (turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
			*/
		}
		break;
	case Preamble:
		data = PREAMBLE_FRAME;
		if(++index >= PREAMBLE_LEN)
		{
			index=0;
			LocoATimeout--;
			LocoBTimeout--;
			TX_state = TX_state_next;

			update = 1;
		}
		break;
	case LocoA:

		switch(Loco_state)
		{
			case Address:
				data = EngineAddress[0][index];
				if(++index >= arrayLen(EngineAddress[0]))
				{
					index = 0;
					Loco_state = Instruction;
				}
				break;
			case Instruction:
				data = EngineInstruction[engineA_sample][index];
				if(++index >= arrayLen(EngineInstruction[0]))
				{
					index = 0;
					Loco_state = Error;
				}
				break;
			case Error:
				data = EngineError[0][engineA_sample][index];
				if(++index >= arrayLen(EngineError[0]))
				{
					Loco_state = Address;
					index=0;
					TX_state=Preamble;
					if(++locoPktCount <= PKT_RESEND_MAX) TX_state_next = LocoA;
					else
					{

						LocoATimeout = IDLE_PAD;
						LocoBTimeout--;
						locoPktCount = 0;

						if(turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
						else if (LocoBTimeout <= 0) TX_state_next = LocoB;
						else if (turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
						else TX_state_next = Idle;
					}

				}
				break;
		}
		break;
	case LocoB:
		switch(Loco_state)
		{
			case Address:
				data = EngineAddress[1][index];
				if(++index >= arrayLen(EngineAddress[1]))
				{
					index = 0;
					Loco_state = Instruction;
				}
				break;
			case Instruction:
				data = EngineInstruction[engineB_sample][index];
				if(++index >= arrayLen(EngineInstruction[1]))
				{
					index = 0;
					Loco_state = Error;
				}
				break;
			case Error:
				data = EngineError[1][engineB_sample][index];
				if(++index >= arrayLen(EngineError[1]))
				{
					Loco_state = Address;
					index=0;
					TX_state=Preamble;

					if(++locoPktCount <= PKT_RESEND_MAX) TX_state_next = LocoB;
					else
					{
						locoPktCount = 0;
						LocoATimeout--;
						LocoBTimeout = IDLE_PAD;

						if (turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
						else if (LocoATimeout <= 0) TX_state_next = LocoA;
						else if(turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
						else TX_state_next = Idle;
					}
				}
				break;
		}
		break;
	case TurnA:
		data = Turnout[0][turnA_sample][index];
		if(++index >= arrayLen(Turnout[0][0]))
		{

			turnA_previous_sample = turnA_sample;
			index=0;
			LocoATimeout--;
			LocoBTimeout--;
			TX_state=Preamble;
			if(LocoBTimeout <= 0) TX_state_next = LocoB;
			//else if(turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
			else if(LocoATimeout <= 0 ) TX_state_next = LocoA;
			else TX_state_next = Idle;

		}
		break;
	case TurnB:
		data = Turnout[1][turnB_sample][index];
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
	if((SPI0->S)&SPI_S_SPTEF_MASK) SPI0->D = data;
	
}

/** Delay function. **/
 void delay(unsigned int d) {
  volatile uint32_t i = d*3*209715u;
  //for (i = 0; i<d; i++)
  while(i-->0)
	{
  }
}






void DCC_init() 
{
	createPackets(); 
	initDCC_withSSI(1);
	initCommandRX_withGPIO(0);
	engineA=8;
	engineB=8;
}
	

void createPackets(void)
{
	uint8_t i,j;
	//Generate packets from strings
	for(i=0;i<2;i++)
	{
		updater(EngineAddress_strings[i],EngineAddress[i],arrayLen(EngineAddress_strings[0]),arrayLen(EngineAddress[0]));
		for(j=0;j<arrayLen(Turnout_strings[0]);j++)
		{
			updater(Turnout_strings[i][j],Turnout[i][j],arrayLen(Turnout_strings[0][0]),arrayLen(Turnout[0][0]));
		}
	}
	for(j=0;j<arrayLen(EngineInstruction_strings);j++)
	{
		updater(EngineInstruction_strings[j],EngineInstruction[j],arrayLen(EngineInstruction_strings[0]),arrayLen(EngineInstruction[0]));
		errorStringGen(EngineAddress_strings[0],EngineInstruction_strings[j],EngineError_strings[0][j]);
		updater(EngineError_strings[0][j],EngineError[0][j],arrayLen(EngineError_strings[0][0]),arrayLen(EngineError[0][0]));

		errorStringGen(EngineAddress_strings[1],EngineInstruction_strings[j],EngineError_strings[1][j]);
		updater(EngineError_strings[1][j],EngineError[1][j],arrayLen(EngineError_strings[0][0]),arrayLen(EngineError[0][0]));

	}
	updater(IdlePacket_string,IdlePacket,arrayLen(IdlePacket_string),arrayLen(IdlePacket));

#ifdef CONSIST
	updater(Consist_strings,ConsistPacket);
#endif
#ifdef REPROGRAM

	updater(FactoryReset_string,FactoryReset,arrayLen(FactoryReset_string),arrayLen(FactoryReset));
	updater(ProgramAddress2_string,ProgramAddress2,arrayLen(ProgramAddress2_string),arrayLen(ProgramAddress2));
	updater(ProgramAddress4_string,ProgramAddress4,arrayLen(ProgramAddress4_string),arrayLen(ProgramAddress4));
#endif
}

