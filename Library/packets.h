/*
 * packets.h
 *
 *  Created on: Mar 10, 2016
 *      Author: amsaclab
 */

#ifndef PACKETS_H_
#define PACKETS_H_


#include <string.h>
#include "constants.h"

const char EngineAddress_strings[2][10] =
//"0000100010":2 (engine A)
//"0000101110":4 (engine B)
{"0000100010","0000101110"};

//Add more speed steps here if necessary, just follow the pattern.
//Speed instructions should have the format: 01DCSSSS0, D = dir, S = speed bits 
const char EngineInstruction_strings[9][9] =
{
//steps used: 7,  14,  21,  28
//Forward speeds: 1, 2-3, 4-5, 6-7
	"011001100","011110000","011011000","011111110",
//Reverse speeds: 1, 2-3, 4-5, 6-7
	"010001100","010110000","010011000","010111110",
//Speed zero
	"010000000"
};

//Error codes are automatically generated at bring-up time
char EngineError_strings[2][arrayLen(EngineInstruction_strings)][9];

FRAME_TYPE EngineError[arrayLen(EngineAddress_strings)][arrayLen(EngineInstruction_strings)][5];
FRAME_TYPE EngineAddress[arrayLen(EngineAddress_strings)][5];
FRAME_TYPE EngineInstruction[arrayLen(EngineInstruction_strings)][5];


//Turnout indices: [0]=A, [1]=B, [x][0] = outer, [x][1] = inner
const char Turnout_strings[2][2][28] =
{
	//.AAAAAAAA.CCCCCCCC.XXXXXXXX.   .AAAAAAAA.CCCCCCCC.XXXXXXXX.
	{"0000010010100001000100011011","0000010010100000000100010011"},
	{"0000010010100001010100011001","0000010010100001110100011101"}
};
FRAME_TYPE Turnout[2][arrayLen(Turnout_strings)][13];


//Provides power to engines without affecting their behavior
 const char IdlePacket_string[28] = "0111111110000000000111111111";
FRAME_TYPE IdlePacket[10];


//TODO: Make reprogram portion below work, it doesn't right now
#ifdef REPROGRAM
/*Configuration variables can be set using the instruction format
 * ...{preamble} 0 {address} 0 {1110CCVV 0 VVVVVVVV 0 DDDDDDDD} 0 {error xor} 1
 * CC: 11 to write byte, 01 to verify (request)
 * V{10}: The bit code for the configuration variable (1 through 1024)
 * D{8}: Value to write to the CV, or in the verified byte the value currently set
 *
 * Error(xor) = address ^ instrByte0 ^ instrByte1 ^ instrByte2
 *
 *
 * This packet must be sent twice (and after a period of 5ms) to correctly write to the CV register
 *
 * The reset CV for a Bachmann (lenz) decoder: CV8=33, brand can be determined by reading CV8. ID=99 indicates Lenz
 *
 * Packets listed below contain preambles to simplify transmission, since space is not a concern
*/

//CC=11, V=8, D=33: {preamble}0{broadcast address}0{1110.1100}0{0000.1000}0{0010.0001}0{11000101}
 const char FactoryReset_string[60] = "111111111111110000001000111011000000010000001000010110000011";
FRAME_TYPE FactoryReset[25];

//CC=11, V=1, D=2: {preamble}0{broadcast address}0{1110.1100}0{0000.0001}0{0000.0010}0{11101111}
 const char ProgramAddress2_string[60] = 	"111111111111110000000000111011000000000010000000100111011111";
FRAME_TYPE ProgramAddress2[28];

//CC=11, V=1, D=4: {preamble}0{broadcast address}0{1110.1100}0{0000.0001}0{0000.0100}0{11101001}
 const char ProgramAddress4_string[60] = 	"111111111111110000000000111011000000000010000001000111010011";
FRAME_TYPE ProgramAddress4[28];


#endif


#ifdef CONSIST

//Consist control tests: deactivation packets

//addr [0] 0: 0 00000000 0 00010011 0 00000000 0 00010011 1
//addr [1] 2: 0 00000010 0 00010011 0 00000000 0 00010001 1
//addr [2] 3: 0 00000011 0 00010011 0 00000000 0 00010000 1
//addr [3] 4: 0 00000100 0 00010011 0 00000000 0 00010111 1

 const char Consist_strings[4][37] =
{
		"0000000000000100110000000000000100111",
		"0000000100000100110000000000000100011",
		"0000000110000100110000000000000100001",
		"0000001000000100110000000000000101111"
};


FRAME_TYPE ConsistPacket[4][14] =
{
	{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}
};
#endif



#endif /* PACKETS_H_ */
