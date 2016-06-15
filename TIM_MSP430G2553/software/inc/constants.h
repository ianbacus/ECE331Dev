/*
 * queue
 *
 *  Created on: Mar 6, 2016
 *      Author: Ian Bacus (ianbacus@gmail.com)
 */

#ifndef __CONSTANTS__
#define __CONSTANTS__
#include <stdint.h> //int definitions
#include <stdbool.h>
#include <stdlib.h> //calloc, free
#include <stdint.h>

#ifdef TIVA
#include <sysctl.h>
#include <gpio.h>
#include <ssi.h>
#include <interrupt.h>
#include <hw_types.h>
#include <hw_memmap.h>
#include <hw_ints.h>
#include <pin_map.h>
#endif

#define arrayLen(x)	(sizeof(x)/sizeof(x[0]))

#define FRAME_TYPE	uint8_t
#define FRAME_SIZE	(sizeof(FRAME_TYPE))
#define FRAME_BITS	(8*FRAME_SIZE)
#define DELAY 10000000
#define IDLE_PAD 50

//Supports 1 to 8 byte shift registers
#define PREAMBLE_FRAME	((FRAME_TYPE)(0x55555555))
#define PREAMBLE_LEN	(4/FRAME_SIZE)

typedef uint8_t engine_t;
typedef enum
{
	Reset,Idle,Preamble, LocoA, LocoB, TurnA, TurnB
} TX_state_t;


#endif


