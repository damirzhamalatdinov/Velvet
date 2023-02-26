/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FIRMWARE_H__
#define __FIRMWARE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stdbool.h"
#include "const.h"

bool isUpdateReady(void);
bool updateFw(UART_HandleTypeDef *huart);
void updateFirmware(void);

#ifdef __cplusplus
}
#endif

#endif /* __FIRMWARE_H__ */