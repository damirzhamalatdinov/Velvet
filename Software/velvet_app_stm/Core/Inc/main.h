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
#define T_SENSOR_Pin GPIO_PIN_2
#define T_SENSOR_GPIO_Port GPIOE
#define DRUGS_PUMP_Pin GPIO_PIN_3
#define DRUGS_PUMP_GPIO_Port GPIOE
#define PAINT_PUMP_Pin GPIO_PIN_4
#define PAINT_PUMP_GPIO_Port GPIOE
#define WIFI_TX_Pin GPIO_PIN_0
#define WIFI_TX_GPIO_Port GPIOA
#define WIFI_RX_Pin GPIO_PIN_1
#define WIFI_RX_GPIO_Port GPIOA
#define WIFI_EN_Pin GPIO_PIN_2
#define WIFI_EN_GPIO_Port GPIOA
#define GSM_TX_Pin GPIO_PIN_10
#define GSM_TX_GPIO_Port GPIOB
#define GSM_RX_Pin GPIO_PIN_11
#define GSM_RX_GPIO_Port GPIOB
#define GSM_ON_STATUS_Pin GPIO_PIN_9
#define GSM_ON_STATUS_GPIO_Port GPIOD
#define GSM_PWRKEY_Pin GPIO_PIN_10
#define GSM_PWRKEY_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_11
#define LED1_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_12
#define LED2_GPIO_Port GPIOD
#define LED3_Pin GPIO_PIN_13
#define LED3_GPIO_Port GPIOD
#define RFID_GPI_Pin GPIO_PIN_14
#define RFID_GPI_GPIO_Port GPIOD
#define RFID_GPO_Pin GPIO_PIN_15
#define RFID_GPO_GPIO_Port GPIOD
#define RFID_TX_Pin GPIO_PIN_6
#define RFID_TX_GPIO_Port GPIOC
#define RFID_RX_Pin GPIO_PIN_7
#define RFID_RX_GPIO_Port GPIOC
#define RFID_EN_Pin GPIO_PIN_8
#define RFID_EN_GPIO_Port GPIOA
#define LORA_TX_Pin GPIO_PIN_9
#define LORA_TX_GPIO_Port GPIOA
#define LORA_RX_Pin GPIO_PIN_10
#define LORA_RX_GPIO_Port GPIOA
#define LORA_RST_Pin GPIO_PIN_3
#define LORA_RST_GPIO_Port GPIOD
#define RTC_SCL_Pin GPIO_PIN_6
#define RTC_SCL_GPIO_Port GPIOB
#define RTC_SDA_Pin GPIO_PIN_7
#define RTC_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
