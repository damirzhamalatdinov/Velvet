/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WIFI_H__
#define __WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stdbool.h"
#include "const.h"

bool isUpdate(UART_HandleTypeDef *huart);
bool updateFw(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_H__ */