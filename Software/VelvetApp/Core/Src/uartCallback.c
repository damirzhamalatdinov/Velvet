/** @file           : uartCallback.c
	* @brief �������� ��� ������� ��������� ���������� �� ���������� UART
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
	* @brief ������� ��������� ���������� �� ���������� �������� �� ���������� UART
	* @param huart - ��������� �� ��������� ���������� UART
  * @retval �����������
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){	
	//if (huart->Instance == USART6) receiveStage = 1;
}
/**
	* @brief ������� ��������� ���������� �� ���������� ����� �� ���������� UART
	* @details � ������������ � ��������� ���������� ������������ ����� ������������
	����� ������� ��� ���������
	* @param huart - ��������� �� ��������� ���������� UART
  * @retval �����������
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
	* @brief ������� ��������� ������ ����� �� RFID ������
  * @param stage - ������ �����
  * @retval �����������
  */
void setReceiveStage (uint8_t stage){
	receiveStage = stage;
}