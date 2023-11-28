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
#include "cmsis_os2.h"

#define ADC_BUSY 1
#define ADC_FREE 0

typedef enum{
	CALIBRATION,
	READ_WEIGHT,
	SET_OFFSET
}AdcMsg_t;
/* Definitions for adcQueue */
extern osMessageQueueId_t adcQueueHandle;
extern osMessageQueueId_t spiQueueHandle;

void readWeightTask(void *argument);
uint8_t getAdcState(void);
void setAdcState(uint8_t state);
float getWeightValByIndex (uint8_t index);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */