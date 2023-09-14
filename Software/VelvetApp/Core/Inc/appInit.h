/* @file           : appInit.h
  * @date 23.02.2023  
  * @author Kamalov Marat  
  */
	
#ifndef __APPINIT_H
#define __APPINIT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "main.h"

void initApp(UART_HandleTypeDef *pEspUART, UART_HandleTypeDef *pRfidUART);

#ifdef __cplusplus
}
#endif

#endif /* __APPINIT_H */