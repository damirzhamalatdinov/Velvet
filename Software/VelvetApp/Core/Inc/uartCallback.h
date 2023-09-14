/* @file           : uartCallback.h
  * @date 03.04.2023  
  * @author Kamalov Marat  
  */
	
#ifndef __UARTCALLBACK_H
#define __UARTCALLBACK_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "main.h"

void setReceiveStage (uint8_t stage);
void setUARTHandlers(UART_HandleTypeDef *pEspUART, UART_HandleTypeDef *pRfidUART);
#ifdef __cplusplus
}
#endif

#endif /* __UARTCALLBACK_H */