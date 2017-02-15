/*
* ECE 331
* Michigan State University
* MTS Library
* Created by: Ian Bacus   (bacusian@msu.edu)
*             Yousef Gtat (gtatyous@msu.edu)
* Revision: Spring 2016, Mar 30 (Ian)
*
* See MTS.h for list and description of functions and variable definitions.
*/
#include "MTS.h"

//Global variables representing the engine states, so that the direction and speed for each can be changed indeppendently \
(ie for the enginexDir or eningexSpeed commands)
int engineA_DSSS=0;
int engineB_DSSS=ENGINE_SEL_MASK;

#ifdef DEFAULT_SYSTEM_CLOCK
#define CLOCK DEFAULT_SYSTEM_CLOCK
#else
#define CLOCK SystemCoreClock
#endif

/*
* Commands for controlling the MTS after it has been initialized
*
*****************************************************************************************************
* Port C: 19 18 17  16       15 14 13  12       11  10  9 8       7 6 5 4       3    2    1    0
*         x  x  ToB ToA      x  x  Lt4 Lt3	     Lt2 Lt1 x x       x x x ES	     Dir  Sp2  Sp1  Sp0
*****************************************************************************************************
* Sp0-2 : 000 = stop    , 001 = slowest   , 111 = fastest
* Dir   :   0 = reverse ,   1 = forward
* ES    :   0 = EngineA ,   1 = EngineB
* Lt1-4 :   0 = red     ,   1 = green
* ToA&B :   0 = outer   ,   1 = inner
*****************************************************************************************************
* Port D:  7   6   5   4        3   2    1   0
*          LS6 LS5 LS4 LS3	     LS2 LS1  x   x
*
* LS# : 0 = no engine present, 1 = engine at sensor
*/

void engine_delay(void)
{
	volatile int i = CLOCK >> 10;
	while(i-->0){}
		
}


void Engine_start(void)
{
	/* Sets EngineA speed to 5 and direction to forward */

	int update;
	//Read: PortC input Register
	engineA_DSSS ^= ENGINE_DIRSPEED_MASK&(engineA_DSSS^(ENGINE_DIR_MASK + 5));
	update = PTC->PDIR ^ engineA_DSSS;
	PTC->PTOR = ENGINE_MASK & update;
	engine_delay();
}

void Engine_stop(void)
{
	/* stops EngineA by setting its speed to zero */

	int update;
	//Read outputs, generate mask, swap it onto the PDOR using toggle register
	engineA_DSSS ^= ENGINE_SPEED_MASK&engineA_DSSS;
	update = PTC->PDIR ^ engineA_DSSS;
	PTC->PTOR = ENGINE_MASK & update;
	engine_delay();
}


void Engine(int speed, int dir)
{
	/* Pass in a speed argument (0-7) and a direction (0=reverse, 1=forward) for EngineA */
	int update;
	//If engine speed between 0 and 7, and direction equal to 0 or 1
	if ( !(speed & ~7) && !(dir & ~1))
	{
		//Read outputs, generate mask, swap it onto the PDOR using toggle register
		engineA_DSSS ^= ENGINE_DIRSPEED_MASK & (engineA_DSSS^(dir<<ENGINE_DIR_SHIFT | speed));
		update = PTC->PDIR ^ engineA_DSSS;
		PTC->PTOR = ENGINE_MASK & update;
		engine_delay();
	}
}


void Engine_speed(int speed)
{
	/* Pass in a speed argument (0-7) for EngineA */
	int update;
	//If engine speed between 0 and 7
	if (!(speed & ~7) )
	{
		//Read outputs, generate mask, swap it onto the PDOR using toggle register
		engineA_DSSS ^= ENGINE_SPEED_MASK &(engineA_DSSS^speed);
		update = PTC->PDIR ^ engineA_DSSS;
		PTC->PTOR = ENGINE_MASK & update;
		engine_delay();
	}
}

void Engine_direction(int dir)
{
	/* Pass in a dir argument (0=reverse, 1=forward) for EngineA */
	int update;
	if ( !(dir & ~1) )
	{
		//Read outputs, generate mask, swap it onto the PDOR using toggle register
		engineA_DSSS ^= ENGINE_DIRSPEED_MASK &(engineA_DSSS^(dir<<ENGINE_DIR_SHIFT));
		update = PTC->PDIR ^ engineA_DSSS;
		PTC->PTOR = ENGINE_MASK & update;
		engine_delay();
	}
}

void EngineB_start(void)
{
	/* Sets EngineB speed to 5 and direction to forward */
	int update;
	//Read outputs, generate mask, swap it onto the PDOR using toggle register
	engineB_DSSS ^= ENGINE_DIRSPEED_MASK & (engineB_DSSS ^ (ENGINE_DIR_MASK + 5));
	update = PTC->PDIR ^ engineB_DSSS;
	PTC->PTOR = ENGINE_MASK & update;
	engine_delay();
}

void EngineB_stop(void)
{
	/* stops EngineB by setting its speed to zero */

	int update;
	//Read outputs, generate mask, swap it onto the PDOR using toggle register
	engineB_DSSS ^= ENGINE_SPEED_MASK &engineB_DSSS;
	update = PTC->PDIR ^ engineB_DSSS;
	PTC->PTOR = ENGINE_MASK & update;
	engine_delay();
}

void EngineB(int speed, int dir)
{
	/* Pass in a speed argument (0-7) and a direction (0=reverse, 1=forward) for EngineB */
	int update;
	if ( !(speed & ~7) && !(dir & ~1))
	{
		//Read outputs, generate mask, swap it onto the PDOR using toggle register
		engineB_DSSS ^= ENGINE_DIRSPEED_MASK & (engineB_DSSS ^(dir<<ENGINE_DIR_SHIFT | speed));
		update = PTC->PDIR ^ engineB_DSSS;
		PTC->PTOR = ENGINE_MASK & update;
		engine_delay();
	}
}

void EngineB_speed(int speed)
{
	/* Pass in a speed argument (0-7) for EngineB */

	int update;
	//If engine speed between 0 and 7
	if (!(speed & ~7) )
	{
		//Read outputs, generate mask, swap it onto the PDOR using toggle register
		engineB_DSSS ^= ENGINE_SPEED_MASK & (engineB_DSSS^speed);
		update = PTC->PDIR ^ engineB_DSSS;
		PTC->PTOR = ENGINE_MASK & update;
		engine_delay();
	}
}

void EngineB_direction(int dir)
{
	/* Pass in a dir argument (0=reverse, 1=forward) for EngineB */
	int update;
	if ( !(dir & ~1) )
	{
		//Read outputs, generate mask, swap it onto the PDOR using toggle register
		engineB_DSSS ^= ENGINE_DIRSPEED_MASK & (engineB_DSSS^(dir<<ENGINE_DIR_SHIFT));
		update = PTC->PDIR ^ engineB_DSSS;
		PTC->PTOR = ENGINE_MASK & update;
		engine_delay();
	}
}



void Lights(int value) //restore the old version
{
	/* Control all four of the lights with a hexadecimal value - bit0=Lt1, bit1=Lt2, bit2=Lt3, bit3=Lt4
	* Lt4 is MSB and Lt1 is LSB, threrefore make sure the DTE is held upside down for proper reading
	* 1s are represented as green and 0s are represented as red
	* e.g. 15=0xF=0b1111 turns all Lights to green
	*       7=0x7=0b0111 turns Lt1, Lt2, and Lt3 to green, but Lt4 to red
	*       5=0x5=0b0101 sets Lt4=0, Lt3=1, Lt2=0, and Lt1=1 on the DTE board
	*/

	if ( 0 <= value && value <= 15)
	{
		//Read outputs, generate mask, swap it onto the PDOR using toggle register
		int reg = PTC->PDIR;
		reg ^=  (value << LIGHT1_SHIFT);
		PTC->PTOR = LIGHTS_MASK &reg;
	}
}



void Light1(int value)
{
	/* Control Light1 without modifying the rest of them (0=green,1=red) */

	if (value == 1) PTC->PSOR = 1ul << LIGHT1_SHIFT;
	else if (value == 0) PTC->PCOR = 1ul << LIGHT1_SHIFT;
	/*else do nothing */

}

void Light2(int value)
{
	/* Control Light2 without modifying the rest of them (0=green,1=red) */

	if (value == 1) PTC->PSOR = 1ul << LIGHT2_SHIFT;
	else if (value == 0) PTC->PCOR = 1ul << LIGHT2_SHIFT;
	/*else do nothing */
}

void Light3(int value)
{
	/* Control Light3 without modifying the rest of them (0=green,1=red) */

	if (value == 1) PTC->PSOR = 1ul << LIGHT3_SHIFT;
	else if (value == 0) PTC->PCOR = 1ul << LIGHT3_SHIFT;
	/*else do nothing */
}

void Light4(int value)
{
	/* Control Light4 without modifying the rest of them (0=green,1=red) */

	if (value == 1) PTC->PSOR = 1ul << LIGHT4_SHIFT;
	else if (value == 0) PTC->PCOR = 1ul << LIGHT4_SHIFT;
	/*else do nothing */
}

void Turnouts(int route)
{
	/* Pass in a route argument (0=outer, 1=inner) for both Turnouts */

	if (route == 1) PTC->PSOR = 3ul << TURNOUT_A_SHIFT;
	else if (route == 0) PTC->PCOR = 3ul << TURNOUT_A_SHIFT;
	/*else do nothing */
}

void TurnoutA(int route)
{
	/* Pass in a route argument (0=outer, 1=inner) for TurnoutA */

	if (route == 1) PTC->PSOR = 1ul << TURNOUT_A_SHIFT;
	else if (route == 0) PTC->PCOR = 1ul << TURNOUT_A_SHIFT;
	/*else do nothing */
}

void TurnoutB(int route)
{
	/* Pass in a route argument (0=outer, 1=inner) for TurnoutB */

	if (route == 1) PTC->PSOR = 1ul << TURNOUT_B_SHIFT;
	else if (route == 0) PTC->PCOR = 1ul << TURNOUT_B_SHIFT;
	/*else do nothing */
}

void Turnouts_toggle(void)
{
	/* Toggle the direction of both Turnouts.
	* Calling this function will switch both Turnouts to the opposite track, inner to outer or vice versa
	* if the Turnouts were in different positions, toggling will not resynchronize them (they would still be different, but they will both toggle)
	*/

	PTC->PTOR = (3ul << TURNOUT_A_SHIFT);
}

void TurnoutA_toggle(void)
{
	/* Toggle the direction of TurnoutA.
	* Calling this function will switch TurnoutA to the opposite track, inner to outer or vice versa
	*/

	PTC->PTOR = (1ul << TURNOUT_A_SHIFT);
}

void TurnoutB_toggle(void)
{
	/* Toggle the direction of TurnoutA.
	* Calling this function will switch TurnoutA to the opposite track, inner to outer or vice versa
	*/

	PTC->PTOR = (1ul << TURNOUT_B_SHIFT);
}




int Sensors(void)
{
	/* Reads the Sensor inputs and return the location
	* e.g. if an Engine is at Sensor2, this function will return 2
	* If there's no Engine, 0 will be returned
	*
	* Warning: This function assumes that there is only one Engine on the TTS
	*/

	//Read: PortD input Register
	int reg = PTD->PDIR;

	//Mask: reg with sensors macros
	//Return: the sensor number at which the Engine is present 
	if (reg & (1ul << SENSOR1_SHIFT)) return 1;
	else if (reg & (1ul << SENSOR2_SHIFT)) return 2;
	else if (reg & (1ul << SENSOR3_SHIFT)) return 3;
	else if (reg & (1ul << SENSOR4_SHIFT)) return 4;
	else if (reg & (1ul << SENSOR5_SHIFT)) return 5;
	else if (reg & (1ul << SENSOR6_SHIFT)) return 6;
	else                                   return 0;
}

int Sensors_binary(void)
{
	/* Reads the Sensor inputs and return the sensors bits as a binary number
	* e.g. if an EngineA is at Sensor2 and EngineB is at Sensro3, this function will return 0b000110=6
	* If there's no Engine, 0 will be returned
	*
	* Warning: This function assumes that there is only one Engine on the TTS
	*/

	//Read: PortD input Register
	int reg = PTD->PDIR;

	//Shift: Sensor1 input to bit 0
	reg >>= SENSOR1_SHIFT;

	//Mask: reg with sensors macros
	reg &= 0x3F;

	//Return: the sensor binary number in the form 0bxxxxxx 
	return reg;
}

int Sensor1(void)
{
	/* Reads Sensor1 input and returns (1=if Engine present, 0=no Engine) */

	//Read: PortD input Register
	int reg = PTD->PDIR;

	//Mask: reg with sensor1 macro
	//Return: 1 if Engine present, 0 if no Engine
	if (reg & (1ul << SENSOR1_SHIFT)) return 1;
	else							  return 0;
}

int Sensor2(void)
{
	/* Reads Sensor2 input and returns (1=if Engine present, 0=no Engine) */

	//Read: PortD input Register
	int reg = PTD->PDIR;

	//Mask: reg with sensor2 macro
	//Return: 1 if Engine present, 0 if no Engine
	if (reg & (1ul << SENSOR2_SHIFT)) return 1;
	else							  return 0;
}

int Sensor3(void)
{
	/* Reads Sensor3 input and returns (1=if Engine present, 0=no Engine) */

	//Read: PortD input Register
	int reg = PTD->PDIR;

	//Mask: reg with sensor3 macro
	//Return: 1 if Engine present, 0 if no Engine
	if (reg & (1ul << SENSOR3_SHIFT)) return 1;
	else							  return 0;
}

int Sensor4(void)
{
	/* Reads Sensor4 input and returns (1=if Engine present, 0=no Engine) */

	//Read: PortD input Register
	int reg = PTD->PDIR;

	//Mask: reg with sensor4 macro
	//Return: 1 if Engine present, 0 if no Engine
	if (reg & (1ul << SENSOR4_SHIFT)) return 1;
	else							  return 0;
}

int Sensor5(void)
{
	/* Reads Sensor5 input and returns (1=if Engine present, 0=no Engine) */

	//Read: PortD input Register
	int reg = PTD->PDIR;

	//Mask: reg with sensor5 macro
	//Return: 1 if Engine present, 0 if no Engine
	if (reg & (1ul << SENSOR5_SHIFT)) return 1;
	else							  return 0;
}

int Sensor6(void)
{
	/* Reads Sensor6 input and returns (1=if Engine present, 0=no Engine) */

	//Read: PortD input Register
	int reg = PTD->PDIR;

	//Mask: reg with sensor6 macro
	//Return: 1 if Engine present, 0 if no Engine
	if (reg & (1ul << SENSOR6_SHIFT)) return 1;
	else							  return 0;
}

/*
* Functions for initializing the MTS
*/

void MTS_init()
{
	MTS_GPIO_init();
	MTS_IRQ_init();
}

void MTS_GPIO_init(void)
{
	/*This function will configure certain DTE-mapped pins on Port C and Port D to work with the MTS */
	uint8_t j;
	// Enable clk to Ports C and D for Train GPIO
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;

	//MUX the appropriate pins to GPIO
	for (j = 0; j <= 4; j++)   PORTC->PCR[j] |= PORT_PCR_MUX(1);	//engine bits (GPIO)
	for (j = 10; j <= 13; j++) PORTC->PCR[j] |= PORT_PCR_MUX(1);	//LEDs (GPIO)
	for (j = 16; j <= 17; j++) PORTC->PCR[j] |= PORT_PCR_MUX(1);	//turnouts (GPIO)
	for (j = 2; j <= 7; j++)   PORTD->PCR[j] |= PORT_PCR_MUX(1); 	//sensors (GPIO)	


	//set data direction
	PTC->PDDR |= 0x33C1F;   //Port C: 0011 0011 1100 0001 1111 = 33C1F, outputs
	PTD->PDDR &= ~0x000FC; //Port D: 0000 0000 0000 1111 1100 = 000FC, inputs
}

void MTS_IRQ_init(void)
{
	/* This function will enable the PORTD clock and enable interrupts for rising edge events on sensor/switch pins
	* without enabling GPIO or selecting pin functions
	*/
	uint8_t j;
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	for (j = 2; j <= 7; j++) PORTD->PCR[j] |= PORT_PCR_IRQC(9); //Rising-edge(9), falling-edge(10), double-edge(11)
	NVIC->ISER[0] |= (1UL << PORTD_IRQn);
}


