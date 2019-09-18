/*
Bootloader using USB
*/

#include "main.h"
#include "USB_Comm.h"
#include "mTimers.h"
#include "usbd_custom_hid_if.h"

#include "update.h"

bool pc_updater_connected;

uint8_t USB_TX_Buffer[CUSTOM_HID_EPOUT_SIZE]; // CUSTOM_HID_EPOUT_SIZE defined in usbd_customhid.c

extern USBD_HandleTypeDef hUsbDeviceFS;

uint8_t timeoutTimer = 0;// timer instance

void USB_Tx(uint8_t command, uint8_t* data) {
	uint8_t status __attribute__((unused));
	uint8_t arrayPtr = 1;

	memset(&USB_TX_Buffer, 0, sizeof(USB_TX_Buffer));

	USB_TX_Buffer[0] = command;
	while (*data != NULL){	
		USB_TX_Buffer[arrayPtr++] = *data;
		data++;		
	}
	
	status = USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, USB_TX_Buffer, sizeof(USB_TX_Buffer));
}

uint8_t ledTimer = 0;
USB_RX_ArrayStruct usb_RX_ArrayStruct;
void USB_Rx(void) {
	
	if(timeoutTimer == 0 ) {
		timeoutTimer = CreateTimer();
	}
		
	while(GetTimer(timeoutTimer) < DELAY_1000_MS || pc_updater_connected == true){
		
		//SetTimer(timeoutTimer, 0);// debug, keeps the bootloader from jumping to application code
		//#warning "remove this clearing of timer"
						
		if (usb_RX_ArrayStruct.messageAvailableFlag){ // message available
			SetTimer(timeoutTimer, 0);
			ServicePacket(usb_RX_ArrayStruct.USB_RX_Array);
			usb_RX_ArrayStruct.messageAvailableFlag = 0;
		}	
	}
}

void SetPC_ConnectStatus(bool status) {
	pc_updater_connected = status;
}

