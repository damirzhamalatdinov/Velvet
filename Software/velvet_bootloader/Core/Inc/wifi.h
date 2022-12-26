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
bool UpdateFW(UART_HandleTypeDef *huart);
// void (uint8_t *buf, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_H__ */