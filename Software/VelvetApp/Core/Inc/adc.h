/* @file           : adc.h
  * @date 19.04.2023  
  * @author Kamalov Marat  
  */
	
#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#define ADC_BUSY 1
#define ADC_FREE 0

void readWeightTask(void *argument);
uint8_t getAdcState(void);
float getWeightValByIndex (uint8_t index);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */