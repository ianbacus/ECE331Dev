/*
* ECE 331
* Michigan State University
* MTS Library
* Created by: Ian Bacus   (bacusian@msu.edu)
*             Yousef Gtat (gtatyous@msu.edu)
* Revision: Spring 2016
*/

/*
* list of Functions:

*	void Engine_start(void): starts EngineA with speed 5 in forward direction
*	void Engine_stop(void): stops EngineA by setting its speed to zero
*   void Engine(int speed, int dir): selects EngineA and passes the speed and direction arguments
*	void Engine_speed(int speed): selects EngineA and passes the speed argument; direction remians intact
*	void Engine_direction(int dir): selects EngineA and passes the direction argument; speed bits remain intact

*	void EngineB_start(void): starts EngineB with speed 5 in forward direction
*	void EngineB_stop(void): stops EngineB by setting its speed to zero
*	void EngineB(int speed, int direction): selects EngineB and passes the speed and direction arguments
*	void Engine_speedB(int speed): selects EngineB and passes the speed argument; direction remians intact
*	void Engine_directionB(int direction): selects EngineB and passes the direction argument; speed bits remain intact

*	void Lights(int val): Displays the binary representation of val (0-15) on the Lights; Lt1=LSB Lt4=MSB
*	void Light1(int val): Turns Light1 green if val=1, red if val=0, else no change
*	void Light2(int val): Turns Light2 green if val=1, red if val=0, else no change
*	void Light3(int val): Turns Light3 green if val=1, red if val=0, else no change
*	void Light4(int val): Turns Light4 green if val=1, red if val=0, else no change

*	void Turnouts(int route): Pass in a route argument (0=outer, 1=inner) for both Turnouts
*	void TurnoutA(int route): Pass in a route argument (0=outer, 1=inner) for TurnoutA
*	void TurnoutB(int route): Pass in a route argument (0=outer, 1=inner) for TurnoutB
*	void Turnouts_toggle(void): Toggles both Turnouts, red>>green or green>>red
*	void TurnoutA_toggle(void): Toggles only TurnoutA, red>>green or green>>red
*	void TurnoutB_toggle(void): Toggles only TurnoutB, red>>green or green>>red

*	int Sensors(void): Returns sensor number at whcih Engine is present (assuming there's only one Engine on TTS)
*   int Sensors_binary(void): Returns a binary number representing the senesors at witch more than one Engine is present
*	int Sensor1(void): Returns 1 if Engine is present at Sensor1, else 0
*	int Sensor2(void): Returns 1 if Engine is present at Sensor2, else 0
*	int Sensor3(void): Returns 1 if Engine is present at Sensor3, else 0
*	int Sensor4(void): Returns 1 if Engine is present at Sensor4, else 0
*	int Sensor5(void): Returns 1 if Engine is present at Sensor5, else 0
*	int Sensor6(void): Returns 1 if Engine is present at Sensor6, else 0

*	void MTS_init(void): Initializes the MTS GPIO and interrupts
*	void MTS_GPIO_init(void): Initializes the MTS GPIO
*	void Debounce_init(void): Prevents interrupts from firing multiple times
*	void MTS_IRQ_init(void): Initializes MTS interrupts
*/


#ifndef MTS_H_GUARD
#define MTS_H_GUARD

#include "MKL25Z4.h"

//External values: send these to DCC module
extern int engineA_DSSS;
extern int engineB_DSSS;

#define ENGINE_SP0_SHIFT        0
#define ENGINE_SP1_SHIFT        1
#define ENGINE_SP2_SHIFT        2

#define ENGINE_DIR_SHIFT		3
#define ENGINE_SEL_SHIFT	 	4
#define ENGINE_DIR_MASK (1<<ENGINE_DIR_SHIFT)
#define ENGINE_SEL_MASK (1<<ENGINE_SEL_SHIFT)
#define ENGINE_SPEED_MASK (0x7)
#define ENGINE_DIRSPEED_MASK (0xf)
#define ENGINE_MASK 		(0x1f)


#define LIGHT1_SHIFT			10
#define LIGHT2_SHIFT			11
#define LIGHT3_SHIFT			12
#define LIGHT4_SHIFT			13

#define LIGHT1_MASK			(1 << LIGHT1_SHIFT)
#define LIGHT2_MASK			(1 << LIGHT2_SHIFT)
#define LIGHT3_MASK			(1 << LIGHT3_SHIFT)
#define LIGHT4_MASK			(1 << LIGHT4_SHIFT)
#define LIGHTS_MASK			(0xf<<LIGHT1_SHIFT)

#define TURNOUT_A_SHIFT			16
#define TURNOUT_B_SHIFT			17

#define SENSOR1_SHIFT			2
#define SENSOR2_SHIFT			3
#define SENSOR3_SHIFT			4
#define SENSOR4_SHIFT			5
#define SENSOR5_SHIFT			6
#define SENSOR6_SHIFT			7

/********** MTS commands **********/
void Engine_start(void);
void Engine_stop(void);
void Engine(int speed, int dir);  //defaults to EngineA
void Engine_speed(int speed);
void Engine_direction(int dir);

void EngineB_start(void);
void EngineB_stop(void);
void EngineB(int speed, int dir);
void EngineB_speed(int speed);
void EngineB_direction(int dir);

void Lights(int);
void Light1(int);
void Light2(int);
void Light3(int);
void Light4(int);

void Turnouts(int route);
void TurnoutA(int route);
void TurnoutB(int route);
void Turnouts_toggle(void);
void TurnoutA_toggle(void);
void TurnoutB_toggle(void);

int Sensors(void);
int Sensors_binary(void);
int Sensor1(void);
int Sensor2(void);
int Sensor3(void);
int Sensor4(void);
int Sensor5(void);
int Sensor6(void);

/********** MTS init **********/
void MTS_init(void);
void MTS_GPIO_init(void);
void MTS_IRQ_init(void);

#endif /* MTS_H_GUARD */

