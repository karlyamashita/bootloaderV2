#include "main.h"
#include "FlashProgram.h"
#include "usbd_custom_hid_if.h"
#include "stm32_hal_legacy.h"

#include "update.h"
#include "hex.h"
#include "USB_Comm.h"

/*Variable used for Erase procedure*/
static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t pageError = 0;
FLASH_ProcessTypeDef flashError;

extern int firmware_name[4]; // part number

//#define FLASH_TYPEPROGRAM_HALFWORD             0x01U  /*!<Program a half-word (16-bit) at a specified address.*/
//#define FLASH_TYPEPROGRAM_WORD                 0x02U  /*!<Program a word (32-bit) at a specified address.*/
//#define FLASH_TYPEPROGRAM_DOUBLEWORD           0x03U  /*!<Program a double word (64-bit) at a specified address*/

//#define FLASH_TYPEERASE_PAGES     0x00U  /*!<Pages erase only*/
//#define FLASH_TYPEERASE_MASSERASE 0x02U  /*!<Flash mass erase activation*/

int FlashPageErase(FLASH_EraseInitTypeDef *eraseStruct) {
	HAL_FLASH_Unlock();
	
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);
		
	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase = eraseStruct->TypeErase;
	EraseInitStruct.Banks = eraseStruct->Banks;
	EraseInitStruct.PageAddress = eraseStruct->PageAddress;
	EraseInitStruct.NbPages = eraseStruct->NbPages;
	
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &pageError) != HAL_OK)
	{ 
		/* 
			Error occurred while page erase. 
			User can add here some code to deal with this error. 
			SectorError will contain the faulty page and then to know the code error on this page,
			user can call function 'HAL_FLASH_GetError()'
		*/
		
		flashError.ErrorCode = HAL_FLASH_GetError();
		
		//Error_Handler();
		return -1;
	}
	
	HAL_FLASH_Lock();
	return 1;
}

int FlashAddress(MEM_T *programData) {

	HAL_FLASH_Unlock();
	
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, programData->member.std_address, (uint32_t)programData->member.data) == HAL_OK)
	{
		//Address = Address + 4;
	}
	else
	{ 
		/* Error occurred while writing data in Flash memory. 
			 User can add here some code to deal with this error */
		
		flashError.ErrorCode = HAL_FLASH_GetError();
		
		//Error_Handler();		
		return -1;
	}

	/* Lock the Flash to disable the flash control register access (recommended
		 to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock(); 
	return 1;
}

int FlashRead(uint32_t address, uint32_t *data) {
	*data = *(uint32_t*)(address);
	
	return 1;
}

FLASH_ProcessTypeDef GetFlashError(void) {
	return flashError;
}

extern MEM_BUF_T hexRecordBuffer; // in hex.c
uint32_t address32Bit = 0;
void WriteBufToMem(void)
{
	int16_t i = 0;
	uint16_t index = 0;
	uint16_t rec_len = 0;
	uint16_t currentRecordNumber = GetRecordNumber();
	
	HAL_FLASH_Unlock();
	for (i = 0; i < currentRecordNumber; i++){
		index = 0;				// location within record of data to write
		rec_len = hexRecordBuffer.record[i].member.size;
		while(index < (rec_len/4)){	
			address32Bit = (hexRecordBuffer.record[i].member.xtd_address << 16) | hexRecordBuffer.record[i].member.std_address;
					
			if(address32Bit >= HARDWARE_VERSION_ADDRESS && address32Bit < APPLICATION_START_ADDRESS) { // Application name goes in this page
				if(firmware_name[index] != (int)0xFFFFFFFF) { // if address has firmware name already then increment to next hex record and continue
					hexRecordBuffer.record[i].member.std_address += 4;
					index += 1;
					continue;
				}
			}

			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address32Bit, (uint32_t)hexRecordBuffer.record[i].member.data[index]) == HAL_OK)
			{
				hexRecordBuffer.record[i].member.std_address += 4;
				index += 1;//next record
			}
			else
			{ 
				/* Error occurred while writing data in Flash memory. 
					 User can add here some code to deal with this error */
				
				flashError.ErrorCode = HAL_FLASH_GetError();
				_Error_Handler(__FILE__, __LINE__);		
				// TODO, return -1
			}
		}
		// erase the last record
		memset(&hexRecordBuffer.record[i].data, 0x0, MAX_MEM_RECORD);											
	}
		
	/* Lock the Flash to disable the flash control register access (recommended to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock(); 
	//TODO, return 1
}	

