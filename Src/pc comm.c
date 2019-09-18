/*	Bootloader Development - PC comm driver file
 *	MPLAB v8.40
 *	Complier C30 v3.20
 *
 *	 	Author. Sam Grove
 *
 *			09-07-09: 	Compiled and verifing program data transfer.
 *						other phy methods can be used but.
 *
 *			04-21-10:	This handles all the PC packet transfers for 
 *						control and information.
 */
 
#include "main.h"
#include "pc_comm.h"

/*
 * Function:        uint16_t AscII2hex(uint8_t input)
 *
 * Overview:        This method will return the hexidecimal equivelant
 *				  of a ASCII character "1,2,3,4,5,6,7,8,9,A,B,C,D,E,F" 
 *				  This is used to format incoming PC data.
 *
 * PreCondition:    Data passed to this method should be validated before
 *				  passing into the method.  If the data is other than 
 *				  the above listed ASCII characters NULL will be returned
 *
 * Input:           input     - An 8-bit ASCII character ranging from 
 *                              0x30-0x39, 0x41-0x46 or 0x61-0x66
 *
 * Output:          uint16_t	   - The hex equivelant (4-bit nibble)
 *							 of the input.  Error bit is bit 15
 *							 if input is not of specified range
 *
 * Side Effects:    None
 *
 */
const uint8_t hex[] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};
uint16_t AscII2hex(uint8_t input)
{
	switch (input-48){
		// input is "0-9" in ascii
		case 0:			case 1:			case 2:			case 3: 		case 4:	
		case 5:			case 6:			case 7: 		case 8:			case 9:
			return hex[input-48];
		//	break;
		// input is "A-F" in ascii
		case 17:		case 18:		case 19:
		case 20:		case 21:		case 22:
			return hex[input-55];
		//	break;
		// input is "a-f" in ascii
		case 49:		case 50:		case 51:
		case 52:		case 53:		case 54:
			return hex[input-87];
		//	break;
		// mask error bits onto the return
		default:
		//	return 0x8080;
			break;
	}	
	// error return type
	return 0x8080;
}

/*
 * Function:        uint16_t Hex2ascII (uint8_t input)
 *
 * Overview:        This method will return the ascII equivelant
 *				  of a hexadecimal byte rangeing from 0x00 -> 0xff 
 *				  This is used to format outgoing data to a PC.
 *
 * PreCondition:    None
 *
 * Input:           input    - An byte of data.
 *
 * Output:          uint8_t	  - The ascII equivelant of the byte of data
 *
 * Side Effects:    None
 *
 */
const uint8_t ascii[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
uint16_t Hex2ascII (uint8_t input)
{
	INT_T result;	
	result.member.lo8 = ascii[((input>>4) & 0x0f)];	/* seems backwards but we are making a int into a string */
	result.member.hi8 = ascii[(input & 0x0f)];		/* uses upper byte as b0 and lower byte as b1 */
	// return the result
	return result.data;
}

