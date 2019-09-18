#ifndef USB_COMM_H
#define USB_COMM_H

#include "main.h"

#define DELAY_1000_MS 1000
#define DELAY_2000_MS 2000
#define DELAY_4000_MS 4000
#define DELAY_5000_MS 5000

typedef struct USB_RX_ArrayStruct {
	uint8_t USB_RX_Array[64];
	uint8_t messageAvailableFlag;
}USB_RX_ArrayStruct;

void USB_Tx(uint8_t command, uint8_t* data);
void USB_Rx(void);
void SetPC_ConnectStatus(bool status);

#endif // USB_COMM_H
