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
extern osSemaphoreId_t readUartSemHandle;

uint8_t receiveBuffer[8] = {0};
uint8_t espMsgReceived = 0;
uint8_t rfidMsgReceived = 0;
extern uint8_t currentCmdESP;
extern uint8_t receiveStage;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == UART4) HAL_UART_Receive_IT(&huart4,receiveBuffer,8);		
	if (huart->Instance == USART6){
		HAL_UART_Receive_IT(&huart6,inputBuffer,1);	
		receiveStage = 1;		
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == UART4) {
		if(currentCmdESP==receiveBuffer[0]){
			if((receiveBuffer[0]==1)&&(receiveBuffer[1]==1)) HAL_NVIC_SystemReset(); //Jump to bootloader 
			else{
				espMsgReceived = 1;
				osSemaphoreRelease(readUartSemHandle);
			}
		}
	}
	if (huart->Instance == USART6){
		if(receiveStage == 1){
			HAL_UART_Receive_IT(&huart6,inputBuffer+1,inputBuffer[0]);
			receiveStage = 2;
		}
		else if (receiveStage == 2){
			rfidMsgReceived = 1;
			osSemaphoreRelease(readUartSemHandle);
		}
	}
}

void readUart(void *argument){//<<<<----- mytest testFunction
	for(;;){
		osSemaphoreAcquire (readUartSemHandle, MAX_DELAY);
		if(espMsgReceived) {
			readEspResponse(receiveBuffer);
			espMsgReceived = 0;
		}
		if(rfidMsgReceived){
			readRfidResponse(inputBuffer);
			rfidMsgReceived = 0;
		}
	}
}