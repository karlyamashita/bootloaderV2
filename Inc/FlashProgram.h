#ifndef FLASH_PROGRAM_H
#define FLASH_PROGRAM_H

#include "main.h"
#include "usbd_custom_hid_if.h"
#include "stm32_hal_legacy.h"
#include "hex.h"

#define APPLICATION_START_ADDRESS 0x08008000
#define HARDWARE_VERSION_ADDRESS 0x08007800

int FlashPageErase(FLASH_EraseInitTypeDef *eraseStruct);
FLASH_ProcessTypeDef GetFlashError(void);
int FlashAddress(MEM_T *programData);
int FlashRead(uint32_t address, uint32_t *data);
void WriteBufToMem(void);
#endif // FLASH_PROGRAM_H
