/** @file : digitalIO.c
  * @date : 13.08.2023  
  * @author Kamalov Marat  
*/
#include "main.h"

/**
  * @brief  This function implements EXTI lines 2-4 detection callbacks.
	* @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == DI0_Pin){
		if(HAL_GPIO_ReadPin(DI0_GPIO_Port, DI0_Pin) == GPIO_PIN_SET)
			HAL_GPIO_WritePin(DI0_LED_GPIO_Port, DI0_LED_Pin, GPIO_PIN_SET);
		else HAL_GPIO_WritePin(DI0_LED_GPIO_Port, DI0_LED_Pin, GPIO_PIN_RESET);
	}
	else if(GPIO_Pin == DI1_Pin){
		if(HAL_GPIO_ReadPin(DI1_GPIO_Port, DI1_Pin) == GPIO_PIN_SET)
			HAL_GPIO_WritePin(DI1_LED_GPIO_Port, DI1_LED_Pin, GPIO_PIN_SET);
		else HAL_GPIO_WritePin(DI1_LED_GPIO_Port, DI1_LED_Pin, GPIO_PIN_RESET);
	}
	else if(GPIO_Pin == DI2_Pin){
		if(HAL_GPIO_ReadPin(DI2_GPIO_Port, DI2_Pin) == GPIO_PIN_SET)
			HAL_GPIO_WritePin(DI2_LED_GPIO_Port, DI2_LED_Pin, GPIO_PIN_SET);
		else HAL_GPIO_WritePin(DI2_LED_GPIO_Port, DI2_LED_Pin, GPIO_PIN_RESET);
	}	
}