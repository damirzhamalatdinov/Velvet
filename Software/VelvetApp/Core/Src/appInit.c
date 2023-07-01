/**
	* @file appInit.c
	* @brief Исходный код функций инициализации ПО
  * @date 23.02.2023  
  * @authors Kamalov Marat  
  */

#include "appInit.h"
#include "app.h"
//#include "cmsis_os2.h"
//#include "rfid.h"

extern TIM_HandleTypeDef htim6;
/**
	* @brief Функция инициализации ПО: запускает аппаратный таймер 6
  * @param None
  * @retval None
  */
void initApp(void){	
	HAL_TIM_Base_Start_IT(&htim6);
};
