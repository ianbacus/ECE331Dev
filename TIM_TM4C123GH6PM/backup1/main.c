/*
 * main.c
 *
 * Created on: Mar 6, 2016
 * 		Author: Ian Bacus (ianbacus@gmail.com)
 */



#include "constants.h"

void initDCC_withSSI(int8_t);
void initCommandRX_withGPIO(int8_t);
size_t updater (const void* ary, void*dest);
void createPackets(void);

engine_t engineA=0, engineB=0;
uint16_t turnouts=0;



void SSI0_IRQHandler(void)
{
	/*
	 * Only used to handle transmit interrupts
	 */
	static TX_state_t TX_state = Preamble,TX_state_next=LocoA;
	static signed char LocoATimeout = 0, LocoBTimeout = 0;
	static engine_t engineA_sample=0, engineB_sample=0;
	static int8_t turnA_sample=0, turnB_sample=0,turnA_previous_sample=-1,turnB_previous_sample=-1,update=1;
	static uint16_t index = 0;
	FRAME_TYPE data = 0x5555;
	if(index == 0)
	{
		index = 0;
	}
	if (update)
	{
		update = 0;

		if(LocoBTimeout < 0) LocoBTimeout = 0;
		if(LocoATimeout < 0) LocoATimeout = 0;
		turnA_previous_sample = turnA_sample;
		turnB_previous_sample = turnB_sample;


		engineA_sample = engineA;
		engineB_sample = engineB;
		turnA_sample = (turnouts&0x2)>>1;
		turnB_sample = turnouts&0x1;

	}
	switch(TX_state)
	{
	case Idle:
		data = IdlePacket[index];
		if(++index >= sizeof(IdlePacket)/FRAME_SIZE)
		{
			index = 0;
			LocoATimeout--;
			LocoBTimeout--;
			if(LocoBTimeout <= 0)
			{
				TX_state = Preamble;
				TX_state_next = LocoB;
			}
			else if(LocoATimeout <= 0)
			{
				TX_state = Preamble;
				TX_state_next = LocoA;
			}
		}
		break;
	case Preamble:
		data = Preamb[index];
		if(++index >= sizeof(Preamb)/FRAME_SIZE)
		{
			index=0;
			LocoATimeout--;
			LocoBTimeout--;
			TX_state= TX_state_next;

			update = 1;
		}
		break;
	case LocoA:
		data = Engine[0][engineA_sample][index];
		if(++index >= sizeof(Engine[0][engineA_sample])/FRAME_SIZE)
		{
			index=0;
 			TX_state=Preamble;
 			LocoATimeout = IDLE_PAD;
 			LocoBTimeout--;
			if(turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
			else if (LocoBTimeout <= 0) TX_state_next = LocoB;
			else if (turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
			else TX_state = Idle;
		}
		break;
	case LocoB:
		data = Engine[1][engineB_sample][index];
		if(++index >= sizeof(Engine[1][engineB_sample])/FRAME_SIZE)
		{
			index=0;
			TX_state=Preamble;
			LocoATimeout--;
			LocoBTimeout = IDLE_PAD;
			if(turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
			else if (LocoATimeout <= 0) TX_state_next = LocoA;
			else if (turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
			else  TX_state = Idle;
		}
		break;
	case TurnA:
		data = Turnout[0][turnA_sample][index];
		if(++index >= sizeof(Turnout[0][0])/FRAME_SIZE)
		{
			index=0;
			LocoATimeout--;
			LocoBTimeout--;
			TX_state=Preamble;
			if(LocoBTimeout <= 0) TX_state_next = LocoB;
			else if(turnB_previous_sample != turnB_sample) TX_state_next = TurnB;
			else if(LocoATimeout <= 0 ) TX_state_next = LocoA;
			else TX_state = Idle;

		}
		break;
	case TurnB:
		data = Turnout[1][turnB_sample][index];
		if(++index >= sizeof(Turnout[1][0])/FRAME_SIZE)
		{
			LocoATimeout--;
			LocoBTimeout--;
			index=0;
			TX_state=Preamble;
			if(LocoATimeout <= 0) TX_state_next = LocoA;
			else if(turnA_previous_sample != turnA_sample) TX_state_next = TurnA;
			else if(LocoBTimeout <= 0 ) TX_state_next = LocoB;
			else TX_state = Idle;
		}
		break;
	}

	SSIIntClear(SSI0_BASE,SSI_TXEOT|SSI_TXFF);
	SSIDataPut(SSI0_BASE,data);
}

void PORTB_IRQHandler(void)
{
	uint16_t pinData = GPIOPinRead(GPIO_PORTB_BASE,0x7f);
	uint16_t flaggedPins = GPIOIntStatus(GPIO_PORTB_BASE,1);
	GPIOIntClear(GPIO_PORTB_BASE,flaggedPins);
 	if(flaggedPins & 0x1F) // updated
	{
		if(pinData&0x10) engineB = pinData&0xf;
		else engineA = pinData&0xf;
	}
	if(flaggedPins & 0x60) //Turnouts updated
	{
		turnouts = (pinData>>5)&3;
	}

}

int main(void)
  {

	//For testing............
	createPackets();
	initDCC_withSSI(1);
	initCommandRX_withGPIO(1);

	while(1)
	{

	}
}

void createPackets(void)
{
	int i,j;
	//Generate packets from strings
	for(i=0;i<2;i++)
	{
		for(j=0;j<(sizeof(Engine_strings[0])/sizeof(Engine_strings[0][0]));j++)
		{
			updater(Engine_strings[i][j],Engine[i][j]);
		}
		for(j=0;j<2;j++)
		{
			updater(Turnout_strings[i][j],Turnout[i][j]);
		}
	}
	updater(IdlePacket_string,IdlePacket);
#ifdef RESET
	updater(ResetPacket_string, ResetPacket);
#endif
#ifdef CONSIST
	updater(Consist_strings,ConsistPacket);
#endif
}

void initDCC_withSSI(int8_t interrupts_enabled)
{
	/* GPIO initialization
	 * Only Pin A5 supports SSI0
	 */
	uint8_t resetIndex = 0;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // DCC Signal Output
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)){}
	GPIOPinTypeSSI(GPIO_PORTA_BASE , GPIO_PIN_5);
	GPIOPinConfigure(GPIO_PA5_SSI0TX);

	/* SSI initialization
	 * Mode 1: allows continuous streaming out of packets through the SSI FIFO, with no pause
	 * 17544 bitrate: allows one bit per 57us or two bits per 114us, allows the transmission of DCC 1s and 0s
	 * 16 bits are sent per transfer. One DCC bit is either 2 SSI bits (1 = 10) or 4 SSI bits (0 = 1100)
	 */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0)){}
	SSIConfigSetExpClk(SSI0_BASE,SysCtlClockGet(),SSI_FRF_MOTO_MODE_1, SSI_MODE_MASTER, 17544, 16);
	SSIEnable(SSI0_BASE); //Enable the SSI module
#ifdef RESET
	//The DCC manual claims that this reset command will power up the locos and clear their volatile memory
	resetIndex = 0;
	while(resetIndex != 8)
	{
		SSIDataPut(SSI0_BASE,ResetPacket[resetIndex]);
		resetIndex++;
	}
#endif
#ifdef CONSIST
	resetIndex = 0;
	while(resetIndex != 8)
	{
		SSIDataPut(SSI0_BASE,ConsistPacket[CONSIST][resetIndex]);
		resetIndex++;
	}
#endif

	if(interrupts_enabled == 1)
	{
		SSIIntEnable(SSI0_BASE,SSI_TXEOT|SSI_TXFF);
		SSIIntRegister(SSI0_BASE,&SSI0_IRQHandler);
		IntPrioritySet(INT_SSI0,0);
		IntEnable(INT_SSI0);
	}
}

void initCommandRX_withGPIO(int8_t interrupts_enabled)
{
	/* Port E: Engine bits and Turnout bits
	 * [... ToA, ToB, ES, Sp2, Sp1, Sp0]
	 * [...   1    1 . 1 	1 	 1 	  1] = 0x3F
	 */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)){}
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, 0x7F);

	if(interrupts_enabled == 1)
	{
		GPIOIntTypeSet(GPIO_PORTB_BASE, 0x7F, GPIO_BOTH_EDGES); //Interrupt edge: either
		GPIOIntRegister(GPIO_PORTB_BASE,&PORTB_IRQHandler); 	//Register an ISR
		IntPrioritySet(GPIO_INT_PIN_0,2);
		GPIOIntEnable(GPIO_PORTB_BASE,0x7F);		 			//GPIO IRQ generation enabled
		IntEnable(GPIO_INT_PIN_0); 								//NVIC IRQ reception enabled
		IntDisable(0x7E);
		//The NVIC obtains summary interrupt reports (one ISR for all port E events) through \
		pin 0. Only Pin 0 is registered to the NVIC, but all necessary pins generate interrupts
	}
}


typedef enum update_t {CLEAR,PARTIAL,PARTIAL_RETURN,RETURN} update_flag_t;
size_t updater (const void* ary, void* dest)
{
	/*
	 * Read a bit string and expand 0's into 0011, and 1's into 01 (for FIFO on SSI, MSb is sent out first)
	 * pack bitwise into 16-bit aligned data structure for SSI frame output
	 */
	uint16_t* result = (uint16_t *)dest;	//result array points to destination argument
	uint8_t* tempary = (uint8_t*)ary; 	//Read characters from the C string
	uint8_t i =0, result_index = 0, ALIGN = 0;
	uint16_t cell = 0;
	result[0] = 0;
	update_flag_t update_flag = CLEAR;

	for(i=0;i<28;i++)
	{
		if(tempary[i] == '0')
		{
			if(update_flag == PARTIAL)
			{
				cell = (cell << 2) | 0x0; //0x3: opposite
				update_flag = PARTIAL_RETURN;
			}
			else cell = (cell << 4) | 0x3; //0xC: opposite
		}
		else if(tempary[i] == '1')
		{
			cell = (cell << 2) | 0x1; //0x2: opposite
		}
		if(update_flag == PARTIAL_RETURN) {}
		else if(cell&(3<<(13+ALIGN))) update_flag = RETURN; //opposite: compare on 1 bit 15-ALIGN
		else if (cell&(3<<(11+ALIGN))) update_flag = PARTIAL; //opposite: compare on 1 bit 13-ALIGN
		switch(update_flag)
		{
			case RETURN:
				result[result_index] = cell;
				result_index++;
				cell=0;
				result[result_index] = 0;
				update_flag = CLEAR;
				ALIGN = 0;
				break;
			case PARTIAL_RETURN:
				result[result_index] = cell;
				result_index++;
				cell=0x3; //Opposite: 0
				result[result_index] = 0;
				update_flag = CLEAR;
				ALIGN = 2;
				break;
			default:
				break;
		}
	}
	//If a frame was not fully packed before the bit string was read completetly, pack in extra preamble bits
	if(cell != 0)
	{
		while(!(cell & (3<<(13+ALIGN) ) ))
		{
			cell = (cell << 2) | 0x1;
		}
		result[result_index]=cell;
		result_index++;
	}

	//To maintain uniform size for the arrays, pack any remaining empty frames with extra preamble bits
	while(result_index < 7)
	{
		result[result_index]=0x5555;
		result_index++;
	}

	return result_index;
}



