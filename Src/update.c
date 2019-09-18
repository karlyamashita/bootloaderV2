#include "main.h"

#include "update.h"
#include "aes.h"
#include "hex.h"
#include "FlashProgram.h"
#include "USB_Comm.h"
#include "GPIO_Ports.h"
#include "RingBuff.h"

/*
• Bootloader resides: 0x8000000-0x4F00.
• Firmware name: 0x08005000 (16 bytes). Once this is written with a blank application part, it becomes permanent so it can only be updated by correct firmware.
• Firmware version: 0x08006000 (16 bytes). This gets erased and written with new version number each time firmware is updated.
• Make sure application code starts at: 0x08006010

We need to reserve some address locations
• IROM1 = start:0x8000000 size:0x4F00 and checked
• ROM1 = start:0x8004F00 size:0x10 and not checked. This is for the bootloader_info[] and hardware_info[].
*/

//These 3 are for bootloader info.
#ifdef BOOTLOADER_ADDRESS0x8008000

__attribute__((section(".ARM.__at_0x08007000"))) 
char bootloader_info[16] = "v1.0.0"; //revision of bootloader project. Reserve 16 bytes

__attribute__((section(".ARM.__at_0x08007020"))) 
char hardware_info[16] = "v1"; //revision of bootloader project. Reserve 16 bytes

//__attribute__((section(".ARM.__at_0x08007F50"))) 
char copyright[64] = "Copyright 2017-2018 Crux Interfacing - Reseda, CA"; //Copyright text. Reserve 64 bytes


// add these two attibutes to the application code with actual part name and version#
__attribute__((section(".ARM.__at_0x08007800"), zero_init)) // this address is within bootloader memory. Reserve address space of 16 bytes with zero_init
int firmware_name[4]; // part number

__attribute__((section(".ARM.__at_0x0801FF00"), zero_init)) // this address is within app code memory. Reserve address space of 16 bytes with zero_init.
//If this revision gets programmed then we know the whole app has been flashed.
int firmware_ver[4]; // version of firmware

#else
__attribute__((section(".ARM.__at_0x08004F00"))) 
char bootloader_info[16] = "v1.0.0"; //revision of bootloader project. Reserve 16 bytes

__attribute__((section(".ARM.__at_0x08004F10"))) 
char hardware_info[16] = "v1"; //revision of bootloader project. Reserve 16 bytes

//__attribute__((section(".ARM.__at_0x08007F50"))) 
char copyright[64] = "Copyright 2017-2018 Crux Interfacing - Reseda, CA"; //Copyright text. Reserve 64 bytes


// add these two attibutes to the application code with actual part name and version#
__attribute__((section(".ARM.__at_0x08005000"), zero_init)) // this address is within bootloader memory. Reserve address space of 16 bytes with zero_init
int firmware_name[16]; // part number

__attribute__((section(".ARM.__at_0x0801FFF0"), zero_init)) // this address is within app code memory. Reserve address space of 16 bytes with zero_init.
//If this revision gets programmed then we know the whole app has been flashed.
int firmware_ver[16]; // version of firmware

#endif // #ifdef BOOTLOADER_ADDRESS0x8008000

int16_t encryptedFileType = -1; // checks to see if a hex file or crx file

#define MAX_RING_BUFFER 64 // the hex record is at most 43 char. We are dropping the ':' but should add a little bit of a headroom.
#define MAX_BUFFER_SIZE 2
RING_BUFF_INFO ringBuffer;
uint8_t hexRecordBuilderArray[MAX_BUFFER_SIZE][MAX_RING_BUFFER] = {0};
int indexPointer = 0;
int hexRecordCount = 0;
uint8_t endOfRecordFlag = 0; // make sure global

FLASH_EraseInitTypeDef flashEraseStruct;

struct AES_ctx ctx;
/*
This is the 128bit key. It is the same key used in the aes_encryption.exe program.
*/
uint8_t key[] = { 0xA4, 0xDD, 0xE2, 0x16, 0x63, 0x51, 0x4A, 0x4D, 0xB1, 0xAB, 0x0E, 0x5D, 0x2C, 0xE2, 0xD7, 0xB7 }; // new guid (A4DDE216-6351-4A4D-B1AB-0E5D2CE2D7B7)
uint16_t fileSize;


/*
Function: check if PC updater up is connected.
Input: none
Output: none
*/
void Update(void)
{	
	PortC_On(LED2_Red_Pin);
	USB_Rx();
	PortC_Off(LED2_Red_Pin);
}

/*
Function: Init after receiving the file size from PC, else decryption will fail
Input: none
Output: none
*/
void InitEncryption(void) {
	AES_init_ctx(&ctx, key);
}

/*
Function: Check each USB command received
Input: none
Output: none
*/
void ServicePacket(uint8_t* command){
	int errorCode = 0;
	uint8_t tempArray[16]; // this hold each 16 bytes of data
	char charArray[16]; // this is for the file size
	uint8_t mArray[63] = {0}; // buffer to hold data of more than 16 bytes.
	uint8_t index = 0; // index pointer

	switch(*command) { // looking at first byte
		case INFO_DEVICE_AVAILABLE:
			USB_Tx(INFO_DEVICE_AVAILABLE, mArray); // ack that interface is present
			SetPC_ConnectStatus(true);// in bootloader.c
			PortC_On(LED2_Red_Pin); // turn on LEDs to indicate in bootloader mode
			PortC_On(LED1_Blue_Pin);
		break;
		case COMMAND_SYNC:
			USB_Tx(COMMAND_SYNC_OK, mArray);// let PC know we are ready
		break;
		case COMMAND_ERASE_CHIP:
			flashEraseStruct.Banks = 1;
			flashEraseStruct.TypeErase = FLASH_TYPEERASE_PAGES;
			flashEraseStruct.PageAddress = APPLICATION_START_ADDRESS;
			flashEraseStruct.NbPages = 52; // todo find number of pages. 0x801ffff - 0x8006000 = 0x1A000. 0x1A000/0x800 = 52. We are also erasing firmware_ver[] at end of flash memory. 
			if(FlashPageErase(&flashEraseStruct) == 1) {
				// indicate that the erase was completed
				USB_Tx(COMMAND_ERASE_OK, mArray);
			} else {
				USB_Tx(COMMAND_ERASE_ERROR, mArray);
			}
		break;
		case COMMAND_ERASE_HARDWARE_VERSION: // TODO, erase page and update Hardware version with PC software
			flashEraseStruct.Banks = 1;
			flashEraseStruct.TypeErase = FLASH_TYPEERASE_PAGES;
			flashEraseStruct.PageAddress = HARDWARE_VERSION_ADDRESS;
			flashEraseStruct.NbPages = 1;
			if(FlashPageErase(&flashEraseStruct) == 1) {
				// indicate that the erase was completed
				USB_Tx(COMMAND_ERASE_OK, mArray);
			} else {
				USB_Tx(COMMAND_ERASE_ERROR, mArray);
			}
		break;
		case COMMAND_COPYRIGHT:
			if(memcmp(command+1, "CRUX", 4) == 0 ) {// look for keyword "CRUX"
				USB_Tx(COMMAND_COPYRIGHT, (uint8_t*)copyright);// return copyright string
			} else {
				USB_Tx(COMMAND_COPYRIGHT, (uint8_t*)"");// return empty string
			}
		break;
		case COMMAND_RESET_PART_NAME:
			flashEraseStruct.Banks = 1;
			flashEraseStruct.TypeErase = FLASH_TYPEERASE_PAGES;
			flashEraseStruct.PageAddress = APPLICATION_START_ADDRESS; // 
			flashEraseStruct.NbPages = 52; // todo find number of pages
			if(FlashPageErase(&flashEraseStruct) == 1) {
				// indicate that the erase was completed
				USB_Tx(COMMAND_ERASE_OK, mArray);
			} else {
				USB_Tx(COMMAND_ERASE_ERROR, mArray);
			}
		break;
		case COMMAND_UPDATE_START:
			USB_Tx(COMMAND_UPDATE_START, mArray);
		break;
		case COMMAND_UPDATE_END:
			// When EOF, bootloader will send COMMAND_UPDATE_COMPLETE in which the PC will send COMMAND_UPDATE_END.	
			USB_Tx(COMMAND_UPDATE_END, mArray);
		break;
		case COMMAND_CANCLE_UPDATE:
			// todo: cancle the update.
			USB_Tx(COMMAND_CANCLE_UPDATE, mArray);
		break;
		case INFO_FILE_SIZE:
			memset(&charArray, 0 , sizeof(charArray));
			memcpy(&charArray, (uint8_t*)command + 1, 6); // shouldn't be more than 6 bytes to read	
			fileSize = atoi(charArray);// convert ascii to hex number
			// Modify the encryption key using the file size.
			key[14] = (fileSize & 0xffff) >> 8;
			key[15] = fileSize & 0xff;			
			InitEncryption(); // now generate ctx
			USB_Tx(INFO_FILE_SIZE, mArray);
		break;
		case INFO_FILE_NAME:
			USB_Tx(INFO_FILE_NAME, mArray);// ack file received
			memcpy(&mArray, command, strlen((char*)command));

			while(mArray[index] != 0){ // go to end of file name
				index++;
			}	
			if (strncmp((char*)&mArray[index-3], "crx", strlen("crx")) == 0){ // now check if last 3 characters (file extension) is "crx"
				encryptedFileType = 1;		// is crx so is an encrypted file	
			}	
		break;
		case INFO_BOOTLOADER:
			USB_Tx(INFO_BOOTLOADER, (uint8_t*)bootloader_info); // send version to PC
		break;
		case INFO_FIRMWARE_NAME:
			if(firmware_name[0] != 0xffffffff){
				USB_Tx(INFO_FIRMWARE_NAME, (uint8_t*)firmware_name); // send interface name to PC
			} else {
				USB_Tx(INFO_FIRMWARE_NAME, (uint8_t*)"BLANK");
			}
		break;
		case INFO_FIRMWARE_VERSION:
			if ((char) firmware_ver[0] == 'v') {
				USB_Tx(INFO_FIRMWARE_VERSION, (uint8_t*)firmware_ver); // send firmware version to PC
			} else {
				USB_Tx(INFO_FIRMWARE_VERSION, (uint8_t*)"**NO FIRMWARE!**");
			}
		break;
		case INFO_HARDWARE_VERSION:
			if ((char) hardware_info[0] == 'v') {
				USB_Tx(INFO_HARDWARE_VERSION, (uint8_t*)hardware_info); // send hardware version to PC
			} else {
				USB_Tx(INFO_FIRMWARE_VERSION, (uint8_t*)"");
			}
		break;
		default: // COMMAND_UPDATE_NEXT 
			
			if(*command != INFO_DATA) return;
		
			if(encryptedFileType == 1) {
			
// this is where we need to take AES encrypted file and decrypt and flash to memory. we need to take each 16 bytes of data and decrypt					
				memcpy(&tempArray, (uint8_t*)command + 1, 16); // The first byte is the command so skip. We should receive 16 bytes, Command + 16 bytes.
													
				AES_ECB_decrypt(&ctx, tempArray); // tempArray is the decrypted data now.

				// now build the hex record
				for(index = 0; index < 16; index++) {
					if(tempArray[index] == ':') {
						indexPointer = 0; // start at index 0 for hexRecordBuilderArray
					} else if(tempArray[index] == '\r') {
						// do nothing
					} else if(tempArray[index] == '\n') {
						endOfRecordFlag = 1; // once we receive a full record		
						DRV_RingBuffPtr__Input(&ringBuffer, MAX_BUFFER_SIZE); // increment write pointer
					} else {
						hexRecordBuilderArray[ringBuffer.iIndexIN][indexPointer++] = tempArray[index];			
					}					
				}					

				if(!endOfRecordFlag) return;
				endOfRecordFlag = 0;
				
				BuildHexRec(&hexRecordBuilderArray[ringBuffer.iIndexOUT][0]);
				memset(&hexRecordBuilderArray[ringBuffer.iIndexOUT][0], 0 , sizeof(hexRecordBuilderArray[ringBuffer.iIndexOUT]));// clear current array
				DRV_RingBuffPtr__Output(&ringBuffer, MAX_BUFFER_SIZE); // increment read pointer
			} else {
				BuildHexRec(command + 2);
			}
	
			errorCode = ValidateHexRec();
			
			if (errorCode == -1){
					USB_Tx(COMMAND_UPDATE_ERROR_CHECKSUM, mArray);
					break;
			}	
			
			errorCode = FormatToProgram();
			
			switch(errorCode) {
				case -1: // END_OF_FILE
					// force update complete
					WriteBufToMem();// TODO, check for good write else send error command									
					// indicate success
					USB_Tx(COMMAND_UPDATE_COMPLETE, mArray);
				break;
				case -2:
					// unknown record type. 
					USB_Tx(COMMAND_UPDATE_BAD_PARSE, mArray);
				break;
				case -3:
					// the file contents are trying to overrite the bootloader section.
					USB_Tx(INFO_BOOTLOADER_OVERWRITE, mArray);
				break;
				default: // errorCode = 1
					// good return - update the record count and program if necessary	
					if(++hexRecordCount == MAX_RECORD_AMT){	// start flashing when maximum records have been saved.
						WriteBufToMem();// TODO, check for good write else send error command					
						// reset the counters
						hexRecordCount = 0;
						SetRecordNumber(0); // clear recordNumber in hex.c
						// ask for the next set of records
						USB_Tx(COMMAND_UPDATE_NEXT, mArray);
					}		
				break;
			}
			break;
	}
}	

