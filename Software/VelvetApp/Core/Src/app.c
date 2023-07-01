 /**
  * @file app.c
	* @brief Исходный код функции запуска задач по таймауту
  * @date 23.02.2023  
  * @author Kamalov Marat  
  */

#include "app.h"
#include "cmsis_os2.h"
#include "esp.h"

volatile uint16_t secondsCounter;
volatile uint32_t timestamp = 0;
static EspMsg_t espMsg;

/**
	* @brief Функция запуска задач по таймауту
	* @details 1 раз в 15 минут вызывается синхронизация системного времени
						 1 раз в минуту вызывается проверка наличия обновлений ПО
  * @param Отсутствует
  * @retval Отсутствует
  */
void checkTasksTimeout(){
	secondsCounter++;
	timestamp++;
	
	if(secondsCounter>TIME_SYNCHRO_TIMEOUT){
		secondsCounter=0;
		espMsg = TIME_SYNCHRONIZATION;
		osMessageQueuePut(espSendQueueHandle, &espMsg, 0, 0);		
	}	
	if(secondsCounter%CHECK_FW_TIMEOUT == 0){
		espMsg = CHECK_FW;
		osMessageQueuePut(espSendQueueHandle, &espMsg, 0, 0);
	}
}
