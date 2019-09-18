#include "main.h"
#include "hex.h"
#include "ascii_to_hex.h"

INT_T xtdAddress;
HEX_REC_T hexRecord;
MEM_BUF_T hexRecordBuffer; // hold multiple hex records
uint16_t recordNumber = 0;

void BuildHexRec(uint8_t* ascHex)
{
	uint16_t writePtr = 0;
	
	// ascii to hex
	while (*ascHex != '\0'){
		hexRecord.data[writePtr] = (AscII2hex(*ascHex) << 4);
		ascHex++;
		hexRecord.data[writePtr] |= AscII2hex(*ascHex);
		ascHex++;
		writePtr++;
	}
}

int16_t ValidateHexRec(void)
{
	uint8_t checkSum = 0;
	uint16_t i = 0;
	uint16_t packet_size = hexRecord.member.length + 5;
	// run the length of the packet and the sum should be 0
	for(i = 0; i < packet_size; i++){
		checkSum += hexRecord.data[i];	
	}	
	if (checkSum == 0){
		return 1;
	}
	return -1;	
}	

/*
parse the type of hex record
*/
int16_t FormatToProgram(void)
{
	uint32_t address = 0;

	// store in RAM as a memory aligned record that can be written in 32bit chunks
	switch(hexRecord.member.rec_type){
		case NORMAL_LINE: // 0x00
			hexRecordBuffer.record[recordNumber].member.xtd_address = xtdAddress.data; // using MEM_T
			hexRecordBuffer.record[recordNumber].member.std_address = hexRecord.member.address_hi << 8 | hexRecord.member.address_lo ;
		
			address = xtdAddress.data << 16 | hexRecordBuffer.record[recordNumber].member.std_address;
		
			if(address < 0x08007800) { // do not allow programming into bootloader space!
				return -3;
			}

			memcpy(hexRecordBuffer.record[recordNumber].data+4, hexRecord.data+4, hexRecord.data[0]);
			hexRecordBuffer.record[recordNumber].member.size = (hexRecord.member.length);
			recordNumber++;
			break;
		case XTD_ADDRESS: // 0x04
			xtdAddress.member.hi8 = hexRecord.data[4];
			xtdAddress.member.lo8 = hexRecord.data[5];
			break;
		case END_OF_FILE: // 0x01
			return -1;
		//	break;	
		case START_LINEAR_ADDRESS: // 0x05
			// do nothing but still return 1
			break;
		default: // unknown record
			return -2;
		//	break;
	}
	return 1;	
}		

void SetRecordNumber(uint16_t data) {
	recordNumber = data;
}

uint16_t GetRecordNumber(void) {
	return recordNumber;
}

