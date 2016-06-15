/*
 * packetGen.h
 *
 *  Created on: Mar 10, 2016
 *      Author: amsaclab
 */

#ifndef PACKETGEN_H_
#define PACKETGEN_H_

#include "constants.h"

#define PARTIALSHIFT	(FRAME_BITS-3)
#define RETURNSHIFT		(FRAME_BITS-5)
typedef enum update_t {CLEAR,PARTIAL,PARTIAL_RETURN,RETURN} update_flag_t;
size_t updater (const void* ary, void* dest, int8_t strlen,int8_t bytelen);


#endif /* PACKETGEN_H_ */
