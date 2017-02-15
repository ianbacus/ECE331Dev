/*
	Code by Ian Bacus (bacusian@msu.edu)
	Summer 2015
*/


#include "Debounce.h"

//Delay based on oscilloscope readings of noise: ~10us is a safe boundary on signal bounce time

//both constants are in microseconds
#define DELAY 5500
int LOCK_DELAY = 20000;


void (*timer_callback) (int);
unsigned int captureA = 0,captureB = 0;
int lockedPins = 0;

volatile struct 
{
	unsigned int pin;
	signed int timeout;
} lockA = {0,0}, lockB = {0,0};


//Set a timeout for each pin
volatile int pinTimeouts[6] = {0,0,0,0,0,0};


volatile enum IRQ_state {TIMER0=(1<<0),TIMER1=(1<<1)} timer_run_next=TIMER0;

/* Timer interrupt service routines here are used as bottom half handlers enqueued by the port D (GPIO) interrupt service routine
 * 
 * A sample of the interrupt flags is used to indicate which pin generated the interrupt request
 * The bottom half handlers inspect the captured ISFR information and compare it with value read from the GPIO input register
 *
 * timer_run_next is used as a flag to indicate which timer should run next. It is alternated on every enqueueing of a timer handler
 */

//Bottom half handler 0
void TPM0_IRQHandler(void)
{
	int temp_captureA,i,b=0;
	TPM0->SC &= ~TPM_SC_CMOD(3); //disable counter
	if(!(PTD->PDIR & captureA) || (captureA&lockedPins))	 //See if the original bit that triggered the event is active now
	{
		//Mismatch indicates that the signal is no longer high after the bounce duration
		//This should not invoke the callback function, this interrupt was triggered by noise
		//captureA=0;
		PORTD->ISFR	|= captureA;
		return; 
	}
	
	//capture A prefers lower numbered sensors
	for(i=4; !(i&captureA) && i<(4<<6); i=i<<1,b++){}
	temp_captureA = i;
		
	if(pinTimeouts[b] > 0) {PORTD->ISFR	|= captureA; return;}
	lockedPins |= temp_captureA;
	pinTimeouts[b]  = LOCK_DELAY;
	
	TPM0->SC |= TPM_SC_TOF_MASK;
	timer_callback(temp_captureA);
	
	PORTD->ISFR	|= captureA;
	captureA = 0;
	
}

//Bottom half handler 1
void TPM1_IRQHandler(void)
{
	int temp_captureB=0,i,b=6;
	TPM1->SC &= ~TPM_SC_CMOD(3);
	if(!(PTD->PDIR & captureB) || (captureB&lockedPins)) 
	{
		PORTD->ISFR	|= captureB;
		return;
	}
	//capture B prefers lower numbered sensors
	for(i=(4<<6); !(i&captureB) && i>(4); i=i>>1,b--){}
	temp_captureB = i;
		
	if(pinTimeouts[b] > 0) {PORTD->ISFR	|= captureB; return;}
	lockedPins |= temp_captureB;
	pinTimeouts[b] = LOCK_DELAY;
	TPM1->SC |= TPM_SC_TOF_MASK;
	timer_callback(temp_captureB);
	
	PORTD->ISFR	|= captureB;
	captureB = 0;
	
}

void TPM2_IRQHandler(void)
{
	int i = 0;
	for(i=0;i<=5;i++)
	{
		if(pinTimeouts[i] == 1)
		{
			pinTimeouts[i]--;
			lockedPins &= ~(1<<(2+i));
		}
		else if(pinTimeouts[i] > 0) pinTimeouts[i]--;
		else pinTimeouts[i] = 0;
	}
	TPM2->SC |= TPM_SC_TOF_MASK;
	TPM2->CNT = 1;
}



void Debounce_init(void (*cb_function) (int)) 
	{
	/* This function configures timers to run for a variable amount of time, they are initially disabled. The debouncing algorithm that uses these
	* timers can be found in TTS_IRQ.c
	*
	* This configuration is designed around the internal reference clock.
	* MCG: enable IR clock. select IR clock. set it to fast IR clock
	* SIM: set TPM source to MCG's IR clock. Enable TPM0 trough TPM2
	* TPM: timer and interrupt information settings, totally abstracted from the clocks
	*/

	timer_callback = cb_function;
	MCG->C1 |= MCG_C1_IREFS_MASK | MCG_C1_IRCLKEN_MASK; 
		//enable and select internal reference clock: internal FLL
	MCG->C2 |= MCG_C2_IRCS_MASK; 
		//Select fast internal reference clock

	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(3); //TPM clock source set to MCGIRCLK
	SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK; //Enable TPM2 clock
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK; //Enable TPM1 clock
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK; //Enable TPM0 clock

	/* Timers 1 and 2 are used by the train as one-shot timers for debouncing
	* sensor handler code is actually placed in TPM0 and TPM1 ISRs
		
		clock source (MCGIRCLK) fast: 4MHz
		PS(2) sets the prescaler to divide by 4: results in 1MHz counting
	*/
	TPM0->CONF |= TPM_CONF_CSOO_MASK;
	TPM0->SC = TPM_SC_CMOD(0) | TPM_SC_PS(3) | TPM_SC_TOIE_MASK;
	TPM0->MOD = 0;

	TPM1->CONF |= TPM_CONF_CSOO_MASK;
	TPM1->SC = TPM_SC_CMOD(0) | TPM_SC_PS(3) | TPM_SC_TOIE_MASK;
	TPM1->MOD = 0;
	

	TPM2->SC = TPM_SC_CMOD(1) | TPM_SC_PS(3) | TPM_SC_TOIE_MASK;
	TPM2->CNT = 1;
	TPM2->MOD = 2;

	NVIC->ISER[0] |= (1UL << TPM0_IRQn) | (1UL << TPM1_IRQn) | (1UL << TPM2_IRQn);
}

void PORTD_IRQHandler(void)
{
	Debouncer();
}

//Handles concurrent interrupts with parallel running timers
void Debouncer(void)
{
//	unsigned char i,b=2;
	unsigned int capture=0,cap=PORTD->ISFR;
	
	//Clean the captured signal so that it only shows one active sensor interrupt
	//for(i=4; !(i&PORTD->ISFR) && b<8; i=i<<1,b++){}
	//capture |= i;
	
	PORTD->ISFR	= cap;
	capture = cap;
	
	//Alternate timers to simulate a queue. Assign a specific interrupt pin to a timer using captureX, re-invoke timer as necessary for that pin 
	
	capture &= ~(lockedPins);
	if(capture == 0)
	{
		capture = 0;
	}
	else if(timer_run_next==TIMER0)
	{
			//lockedPins |= capture;
			timer_run_next = TIMER1;
			captureA = capture;
			TPM0->CNT = 1;
			TPM0->MOD = DELAY; //set timer0 to run for 10us
			TPM0->SC |= TPM_SC_CMOD(1); //enable LPTMR counter for timer0
	}
	else if(timer_run_next==TIMER1)
	{
			timer_run_next = TIMER0;
			captureB = capture;
			TPM1->CNT = 1;
			TPM1->MOD = DELAY; //set timer0 to run for 10us
			TPM1->SC |= TPM_SC_CMOD(1); //enable LPTMR counter for timer1
	}
}
