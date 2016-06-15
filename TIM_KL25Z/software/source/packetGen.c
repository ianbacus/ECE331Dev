/*
 * packetGen.c
 *
 *  Created on: Mar 10, 2016
 *      Author: amsaclab
 */


#include "packetGen.h"


size_t updater (const void* ary, void* dest, int8_t strlen,int8_t bytelen)
{
	/*
	 * Read a bit string and expand 0's into 0011, and 1's into 01 (for FIFO on SSI, MSb is sent out first)
	 * pack bitwise into 16-bit aligned data structure for SSI frame output
	 */
	update_flag_t update_flag = CLEAR;
	FRAME_TYPE* result = (FRAME_TYPE *)dest;	//result array points to destination argument
	uint8_t* tempary = (uint8_t*)ary; 	//Read characters from the C string
	uint8_t i =0, result_index = 0, ALIGN = 0;
	FRAME_TYPE cell = 0;
	result[0] = 0;


	for(i=0;i<strlen;i++)
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
		else if(cell&(3<<(PARTIALSHIFT+ALIGN))) update_flag = RETURN; //opposite: compare on 1 bit 15-ALIGN
		else if (cell&(3<<(RETURNSHIFT+ALIGN))) update_flag = PARTIAL; //opposite: compare on 1 bit 13-ALIGN
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
		while(!(cell & (3<<(PARTIALSHIFT+ALIGN) ) ))
		{
			cell = (cell << 2) | 0x1;
		}
		result[result_index]=cell;
		result_index++;
	}

	//To maintain uniform size for the arrays, pack any remaining empty frames with extra preamble bits
	while(result_index < bytelen)
	{
		result[result_index]=PREAMBLE_FRAME;
		result_index++;
	}

	return result_index;
}


