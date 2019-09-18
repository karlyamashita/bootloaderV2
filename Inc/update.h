#ifndef UPDATE_H
#define UPDATE_H
	
#include "main.h"
	
#define COMMAND_SYNC 0x3F
#define COMMAND_SYNC_OK 0x40

#define COMMAND_UPDATE_START 0x41
#define COMMAND_UPDATE_NEXT 0x42
#define COMMAND_UPDATE_COMPLETE 0x43
#define COMMAND_UPDATE_OK 0x44
#define COMMAND_UPDATE_BAD_PARSE 0x45
#define COMMAND_UPDATE_ERROR 0x46
#define COMMAND_UPDATE_END 0x47
#define COMMAND_UPDATE_ERROR_CHECKSUM 0x48

#define COMMAND_ERASE_CHIP 0x49
#define COMMAND_ERASE_OK 0x4A
#define COMMAND_ERASE_HARDWARE_VERSION 0x67
#define COMMAND_ERASE_ERROR 0x66

#define INFO_FILE_SIZE 0x4B
#define INFO_FILE_NAME 0x4C

#define INFO_FIRMWARE_NAME 0x4D
#define INFO_FIRMWARE_VERSION 0x4E

#define COMMAND_CANCLE_UPDATE 0x4F
#define INFO_BOOTLOADER 0x50

#define INFO_HARDWARE_VERSION 0x53
#define INFO_BOOTLOADER_OVERWRITE 0x54

#define INFO_DEVICE_AVAILABLE 0x60
#define INFO_DATA 0x65
#define INFO_APPLICATION_MODE 0x70

#define COMMAND_RESET_PART_NAME 0x77
#define COMMAND_RESET 0x80

#define COMMAND_COPYRIGHT 0x90

void InitEncryption(void);
void Update(void);
void ServicePacket(uint8_t*);
		
#endif // UPDATE_H