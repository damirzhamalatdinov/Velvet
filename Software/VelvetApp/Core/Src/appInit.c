/* @file           : appInit.c
  * @date 23.02.2023  
  * @author Kamalov Marat  
  */

#include "appInit.h"
#include "app.h"
#include "uartCallback.h"
//#include "cmsis_os2.h"
//#include "rfid.h"

extern TIM_HandleTypeDef htim6;

void initApp(UART_HandleTypeDef *pEspUART, UART_HandleTypeDef *pRfidUART){	
	HAL_TIM_Base_Start_IT(&htim6);
	setUARTHandlers(pEspUART, pRfidUART);	
};
