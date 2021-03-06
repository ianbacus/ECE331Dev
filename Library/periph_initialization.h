/*
 * periph_initialization.h
 *
 *  Created on: Mar 10, 2016
 *      Author: amsaclab
 */

#ifndef PERIPH_INITIALIZATION_H_
#define PERIPH_INITIALIZATION_H_

#include "MKL25z4.h"
#include "constants.h"

void initDCC_withSSI(int8_t interrupts_enabled);
void initCommandRX_withGPIO(int8_t interrupts_enabled);
void SSIDataPut(FRAME_TYPE data);

#endif /* PERIPH_INITIALIZATION_H_ */
