/* @file           : uartCallback.c
* @date 03.04.2023  
* @author Kamalov Marat  
*/
#include "uartCallback.h"
#include "stm32f4xx_hal.h"
#include "rfid.h"
#include "esp.h"
#include "cmsis_os2.h"
#include "app.h"

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart6;
extern osMessageQueueId_t espReceiveQueueHandle;
extern osMessageQueueId_t rfidReceiveQueueHandle;

static const uint8_t espMsgReceived = 1;
static uint8_t receiveStage = 0;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){	
	//if (huart->Instance == USART6) receiveStage = 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == UART4) osMessageQueuePut(espReceiveQueueHandle, &espMsgReceived, 0, 0);	 
	if (huart->Instance == USART6){
		if(receiveStage == 1){						
			osMessageQueuePut(rfidReceiveQueueHandle, &receiveStage, 0, 0);
			receiveStage = 2;
		}
		else if (receiveStage == 2){
			osMessageQueuePut(rfidReceiveQueueHandle, &receiveStage, 0, 0);
			receiveStage = 0;
		}
	}
}

void setReceiveStage (uint8_t stage){
	receiveStage = stage;
}