/* @file           : appInit.c
  * @date 23.02.2023  
  * @author Kamalov Marat  
  */

#include "appInit.h"
#include "app.h"
//#include "cmsis_os2.h"
//#include "rfid.h"

extern TIM_HandleTypeDef htim6;

void initApp(void){	
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_TIM_Base_Start_IT(&htim6);
};
