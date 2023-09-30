/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DI2_Pin GPIO_PIN_2
#define DI2_GPIO_Port GPIOE
#define DI2_EXTI_IRQn EXTI2_IRQn
#define DI1_Pin GPIO_PIN_3
#define DI1_GPIO_Port GPIOE
#define DI1_EXTI_IRQn EXTI3_IRQn
#define DI0_Pin GPIO_PIN_4
#define DI0_GPIO_Port GPIOE
#define DI0_EXTI_IRQn EXTI4_IRQn
#define DI2_LED_Pin GPIO_PIN_5
#define DI2_LED_GPIO_Port GPIOE
#define DI1_LED_Pin GPIO_PIN_6
#define DI1_LED_GPIO_Port GPIOE
#define DI0_LED_Pin GPIO_PIN_13
#define DI0_LED_GPIO_Port GPIOC
#define RFID_EN_Pin GPIO_PIN_6
#define RFID_EN_GPIO_Port GPIOA
#define RFID_HEAT_Pin GPIO_PIN_7
#define RFID_HEAT_GPIO_Port GPIOA
#define ESP_EN_Pin GPIO_PIN_15
#define ESP_EN_GPIO_Port GPIOE
#define ADC_EN_Pin GPIO_PIN_12
#define ADC_EN_GPIO_Port GPIOB
#define ADC_DOUT_Pin GPIO_PIN_14
#define ADC_DOUT_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_11
#define LED_GPIO_Port GPIOD
#define DO5_Pin GPIO_PIN_3
#define DO5_GPIO_Port GPIOD
#define DO4_Pin GPIO_PIN_4
#define DO4_GPIO_Port GPIOD
#define DO3_Pin GPIO_PIN_5
#define DO3_GPIO_Port GPIOD
#define DO2_Pin GPIO_PIN_6
#define DO2_GPIO_Port GPIOD
#define DO1_Pin GPIO_PIN_7
#define DO1_GPIO_Port GPIOD
#define DO0_Pin GPIO_PIN_4
#define DO0_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
