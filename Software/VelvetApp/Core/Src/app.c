 /* @file           : app.c
  * @date 23.02.2023  
  * @author Kamalov Marat  
  */

#include "app.h"
#include "cmsis_os2.h"

extern osSemaphoreId_t sendMsgSemHandle;

volatile uint16_t secondsCounter;
uint8_t checkFW = 0;
uint8_t timeSynchronization = 0;
volatile uint32_t timestamp = 0;

void checkTasksTimeout(){
	secondsCounter++;
	timestamp++;
	if(secondsCounter%CHECK_FW_TIMEOUT == 0) checkFW = 1;
	if(secondsCounter>TIME_SYNCHRO_TIMEOUT){
		secondsCounter=0;
		timeSynchronization = 1;
	}
	if(checkFW||timeSynchronization) osSemaphoreRelease(sendMsgSemHandle);
}
