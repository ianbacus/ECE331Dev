/*
 * periph_initialization.h
 *
 *  Created on: Mar 10, 2016
 *      Author: amsaclab
 */

#ifndef PERIPH_INITIALIZATION_H_
#define PERIPH_INITIALIZATION_H_

#include "msp430g2553.h"
#include "constants.h"

void initDCC_withSSI(int8_t interrupts_enabled);
void initCommands_withPORT1(void);

void SSIDataPut(FRAME_TYPE data);
void init_comparator(void);


#endif /* PERIPH_INITIALIZATION_H_ */
