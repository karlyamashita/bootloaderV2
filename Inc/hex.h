#ifndef HEX_H
#define HEX_H

#include "main.h"

#define MAX_HEX_SIZE	21
typedef union HEX_REC_T{
	uint8_t data[MAX_HEX_SIZE];
	struct{
		uint8_t length;
		uint8_t address_hi;
		uint8_t address_lo;
		uint8_t rec_type;
		uint8_t data0;
		uint8_t data1;
		uint8_t data2;
		uint8_t data3;
		uint8_t data4;
		uint8_t data5;
		uint8_t data6;
		uint8_t data7;
		uint8_t data8;
		uint8_t data9;
		uint8_t data10;
		uint8_t data11;
		uint8_t data12;
		uint8_t data13;
		uint8_t data14;
		uint8_t data15;
		uint8_t cksum;
	}member;
}HEX_REC_T;
		
#define MAX_MEM_RECORD	21
typedef union MEM_T{
	uint8_t data[MAX_MEM_RECORD];
	struct{
		uint16_t xtd_address;
		uint16_t std_address;
		uint32_t data[4];
		uint8_t size;
	}member;	
}MEM_T;	

#define MAX_RECORD_AMT	32
typedef struct MEM_BUF_T{
	MEM_T record[MAX_RECORD_AMT];
}MEM_BUF_T;

//#define EOF				0xff
#define END_OF_FILE		0x01
#define XTD_ADDRESS		0x04
#define START_LINEAR_ADDRESS 0x05
#define NORMAL_LINE		0x00
#ifndef TRUE
#define TRUE			0x1
#define FALSE			0x0
#endif


void BuildHexRec(uint8_t*);
int16_t ValidateHexRec(void);
int16_t FormatToProgram(void);
void SetRecordNumber(uint16_t data);
uint16_t GetRecordNumber(void);
#endif

