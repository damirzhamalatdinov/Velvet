/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

float HX711_Read (void); // усиление 128

uint16_t GetAnimalWight();

void ScalesCalibrate();

#ifdef __cplusplus
}
#endif

#endif /* __ADC__ */