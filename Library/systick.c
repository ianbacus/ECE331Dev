/*  ECE 331, FS2015
 *  Michigan State University
 *  Code by: Yousef Gtat (gtatyous@msu.edu)
 *           Ian Bacus   (bacusian@msu.edu)
 */

#include "systick.h"

//call back function for stop_flashing and systic_handler
void (*systic_handler_callback) (void);
int systic_callback_flag = 1;


/************ Internal Functions ************/
void random_toggles (void);  
void random_output (void);
/************************************************/


/************ Genral Systic Functions ************/
void systick_init (int reload)
{
  //use to initialize the systic timer. 
  //use system clock speed / 4 to fire systick interrupt each second
	
	SysTick_RVR_REG(SysTick_BASE_PTR) = SysTick_RVR_RELOAD(reload);
	SysTick_CVR_REG(SysTick_BASE_PTR) = SysTick_CVR_CURRENT(0);
	SysTick_CSR_REG(SysTick_BASE_PTR) |= SysTick_CSR_ENABLE_MASK | SysTick_CSR_TICKINT_MASK | SysTick_CSR_CLKSOURCE_SHIFT;
}

void systick_pause (void)
{
	SysTick_CSR_REG(SysTick_BASE_PTR) &= ~SysTick_CSR_ENABLE_MASK;
}

void systick_resume (void)
{
	SysTick_CSR_REG(SysTick_BASE_PTR) |= SysTick_CSR_ENABLE_MASK;
}

void SysTick_Handler()
{
	//gets called when a systic timer interrupt is fired
	systic_handler_callback();
}
/************************************************/
void set_callback ( void (*f)(void) )
{
  systic_handler_callback = f;
	systic_callback_flag = 0;
}

void stop_flash (void)
{
	systick_pause();
	random_output();  //controls the output from the lights
}

void start_flash (void)
{
	if (systic_callback_flag)
	{
	  systic_handler_callback = random_toggles;
		systic_callback_flag = 0;
	}
	systick_resume();
}

/************ Internal Functions ************/
void random_toggles (void)         
{
	static int x = 0;
	if (x) 
	{ PTC->PCOR = (0x5 << 10);
		PTC->PSOR = (0xA << 10);
		x=0;
	}
	else 
	{
	  PTC->PCOR = (0xA << 10);
		PTC->PSOR = (0x5 << 10);
		x=1;
	}
}

void random_output (void)
{
	static int occurrence = 2; 
  static int LT0_count, LT1_count, LT2_count, LT3_count = 0;
	//time_t t;
	int random_number;
	
	//srand((unsigned) time(&t));
	random_number = rand() % 4;
	
	if      (random_number == 0) LT0_count ++;
	else if (random_number == 1) LT1_count ++;
	else if (random_number == 2) LT2_count ++;
	else 						             LT3_count ++;
	
	/* The following block of code prevent repeated values from occuring */
	if      (LT0_count == occurrence) 
	{ LT0_count = 0; random_number++; }
	else if (LT1_count == occurrence) 
	{ LT1_count = 0; random_number--; }
	else if (LT2_count == occurrence) 
	{ LT2_count = 0; random_number++; }
	else if (LT3_count == occurrence) 
	{ LT3_count = 0; random_number--; }
	
	PTC->PCOR = (0xF << 10);
  PTC->PSOR = ((1ul << random_number) << 10);
}
