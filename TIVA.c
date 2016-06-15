/*
 *
 * TIM ARM SOFTWARE
 * 1/14/14
 * The purpose of this software is to interact with a student
 * micro controller in the ECE331 lab and translate simple commands
 * from the student into more complex DCC signals for controlling a model train
 *
 * Written by Nolan Holmes
 * contact: nol.holmes@gmail.com
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"



//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif
/******************************************************************************/
 /*
  * function declaration
  */
void Preamble(void);
void CreateZero(void);
void CreateOne(void);
void UpdateTurnouts(int portD);
void UpdateTrains(int portE);

//*****************************************************************************
// TURNOUT DCC SIGNAL COMMANDS!!!
// THESE ARE PROPRIETARY TO BACHMANN TURNOUTS!
//*****************************************************************************
char turnout_addrA_open[28] = "0000010010100001000100011011";
//char turnout_addrA_close[28] ="0000010010100001000100011011";
char turnout_addrA_close[28]  = "0000010010100000000100010011";
char turnout_addrB_open[28] = "0000010010100001110100011101";
char turnout_addrB_close[28]  = "0000010010100001010100011001";

//*****************************************************************************
// TRAIN ADDRESSES FOLLOW THE DCC STANDARD!
//*****************************************************************************

/******************************************************************************/
// current state of turnouts stored here. for reference in turnout functions
/******************************************************************************/
int turnout_addrA_state = 0;
int turnout_addrB_state = 0;
/******************************************************************************/
// values to update the train with are stored here
/******************************************************************************/
int train_state = 0;
char train_addr2    = 0x02;
char train_addr4    = 0x04;
volatile char train_command2 = 0x40;
volatile char train_command4 = 0x40;
char train_error2   = 0;
char train_error3   = 0;

//#define DCC_ONE 745 // delays for 57 us
//#define DCC_ZERO 1390 // delays for 107 us
#define DCC_ONE 2*725 // delays for 57 us
#define DCC_ZERO 2*1370 // delays for 107 us

int
main(void)

{

  int turnout_update = 0;
  int train_update = 0;

  // Setup the system clock to run at 80 Mhz from PLL with crystal reference
    SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|
                    SYSCTL_OSC_MAIN);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); // DCC Signal Output
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); // speed, direction, train select bits
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); // turnout select bits
    SysCtlDelay(5);                // Delay for a few seconds while peripherals enable

    GPIOPinTypeGPIOOutput( GPIO_PORTB_BASE , GPIO_PIN_0 );
    // GPIO_PORT_E is input port for speed, train select, and direction
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
        GPIO_PIN_4 | GPIO_PIN_5);
    // GPIO_PORT_D is input port for the track turnout selects
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x00  ); // force DCC signal low until needed

    //
    // Loop Forever
    while(1)
    {
      /*
      Preamble();
      CreateZero();
      CreateZero();
      SysCtlDelay(100000);
      //train_update = 0xFF;
       */


      turnout_update = GPIOPinRead( GPIO_PORTD_BASE, GPIO_PIN_0|GPIO_PIN_1 );

      train_update = GPIOPinRead( GPIO_PORTE_BASE, GPIO_PIN_1|GPIO_PIN_2|
          GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);

      UpdateTrains(train_update);

      //SysCtlDelay(100000);

      UpdateTurnouts( turnout_update );



    }
}


/*
 * preamble is a universal preamble that must precede any DCC signal
 * to the trains. It is 14 logic '1' values to the decoder chip
 * on the trains.
 */

void Preamble(void) {
  int i;
  for(i = 0; i < 16; i++) {
    CreateOne();
  }
}
/*
 * Creates DCC Zero
 */
void CreateZero(void) {
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0  ); // high signal
  SysCtlDelay(DCC_ZERO); //101 usecond delay
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x00 ); // low signal
  SysCtlDelay(DCC_ZERO); //101 usecond delay
}

/*
 * Creates DCC One
 */

void CreateOne(void) {
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0  ); // high signal
  SysCtlDelay(DCC_ONE); //57 usecond delay
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x00 ); // low signal
  SysCtlDelay(DCC_ONE); //57 usecond delay
}

/*
 * Updates DCC turnouts.
 * If turnout has not change, new signal will not be sent
 */

void UpdateTurnouts(int portD) {
  int j; //simple counter variable
  if( ((portD&0x0002) != turnout_addrB_state) ) {
    turnout_addrB_state = portD&0x0002; //turnout B
    if( (turnout_addrB_state > 0) ) {
      Preamble();
      for( j = 0; j < 28 ; j++ ) {
        if (turnout_addrB_close[j] == '1' ) { CreateOne(); }
        else { CreateZero(); }
      }
    }
    else {
      Preamble();
      for( j = 0; j < 28 ; j++ ) {
        if (turnout_addrB_open[j] == '1' ) { CreateOne(); }
        else { CreateZero(); }
      }
    }
  }
  SysCtlDelay(50); // slight delay between tracks
  if( (portD&0x0001) != turnout_addrA_state ) {
    turnout_addrA_state = portD&0x0001;//turnout A
    if( (turnout_addrA_state > 0) ) {
      Preamble();
      for( j = 0; j < 28 ; j++ ) {
        if (turnout_addrA_close[j] == '1' ) { CreateOne(); }
        else { CreateZero(); }
      }
    }
    else {
      Preamble();
      for( j = 0; j < 28 ; j++ ) {
        if (turnout_addrA_open[j] == '1' ) { CreateOne(); }
        else { CreateZero(); }
      }
    }
  }
}

 void UpdateTrains(int portE) {
   char shiftTemp = 0x00;
   int dccCount;
   char shifter = 0x80;
   // Update train state with portE
   //train_state = portE;
     // train command has changed
   shiftTemp = portE&0x10;   // get the direction bit
   shiftTemp = shiftTemp << 1; // shift direction bit one left
   /* AND portE w/ 0E to get 0000XXX0, which is speed */
   shiftTemp = shiftTemp|(portE&0x0E); // add speed bits to tempShift
   if( portE&0x20 ) {
     //train 4 selected
     train_command4 = 0x40;
     train_command4 = train_command4|shiftTemp; // speed bits and direction added to train command
     train_error3 = train_command4^train_addr4;
     // shift direction bit into place
   }
   else
   {
     // do train 2
     train_command2 = 0x40;
     train_command2 = train_command2|shiftTemp; // speed bits and direction added to train command
     train_error2 = train_command2^train_addr2;
   }
   // else do not change either train command

   // now bit bang both commands out
   // Train 2!
   Preamble();
   CreateZero();
   for( dccCount = 0; dccCount < 8; dccCount++) {
     if(shifter&train_addr2) { CreateOne(); }
     else { CreateZero(); }
     shifter = shifter >> 1;
   }
   CreateZero();
   shifter = 0x80;
   for( dccCount = 0; dccCount < 8; dccCount++) {
     if(shifter&train_command2) { CreateOne(); }
     else { CreateZero(); }
     shifter = shifter >> 1;
   }
   CreateZero();
   shifter = 0x80;
   for( dccCount = 0; dccCount < 8; dccCount++) {
     if(shifter&train_error2) { CreateOne(); }
     else { CreateZero(); }
     shifter = shifter >> 1;
   }
   CreateOne(); // ending bit
   // Train 3!
   SysCtlDelay(10); // delay for a bit to see different signals on scope
   shifter = 0x80;
   Preamble();
   CreateZero();
   for( dccCount = 0; dccCount < 8; dccCount++) {
     if(shifter&train_addr4) { CreateOne(); }
     else { CreateZero(); }
     shifter = shifter >> 1;
   }
   CreateZero();
   shifter = 0x80;
   for( dccCount = 0; dccCount < 8; dccCount++) {
     if(shifter&train_command4) { CreateOne(); }
     else { CreateZero(); }
     shifter = shifter >> 1;
   }
   CreateZero();
   shifter = 0x80;
   for( dccCount = 0; dccCount < 8; dccCount++) {
     if(shifter&train_error3) { CreateOne(); }
     else { CreateZero(); }
     shifter = shifter >> 1;
   }
   CreateOne(); // ending bit


 }
