/* @file           : app.h
  * @date 23.02.2023  
  * @author Kamalov Marat  
  */
	
#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#include "stm32f4xx_hal.h"

#define CHECK_FW_TIMEOUT 60
#define TIME_SYNCHRO_TIMEOUT 900
#define MAX_DELAY 0xffff

void checkTasksTimeout(void);

extern volatile uint32_t timestamp;

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */