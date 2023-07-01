/** @file           : uartCallback.c
	* @brief Исходный код функции обработки прерываний по интерфейсу UART
	* @date 03.04.2023  
	* @author Kamalov Marat  
*/
#include "uartCallback.h"
#include "stm32f4xx_hal.h"
#include "rfid.h"
#include "esp.h"
#include "cmsis_os2.h"
#include "app.h"

static const uint8_t espMsgReceived = 1;
static uint8_t receiveStage = 0;
/**
	* @brief Функция обработки прерываний по завершению передачи по интерфейсу UART
	* @param huart - Указатель на структуру интерфейса UART
  * @retval Отсутствует
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){	
	//if (huart->Instance == USART6) receiveStage = 1;
}
/**
	* @brief Функция обработки прерываний по завершению приёма по интерфейсу UART
	* @details В соответствии с выбранным интерфесом производится вызов обработчиков
	через очередь для сообщений
	* @param huart - Указатель на структуру интерфейса UART
  * @retval Отсутствует
  */
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
/**
	* @brief Функция установки стадии приёма от RFID модуля
  * @param stage - стадия приёма
  * @retval Отсутствует
  */
void setReceiveStage (uint8_t stage){
	receiveStage = stage;
}