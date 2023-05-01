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

extern uint8_t adcConversionInProcess;
extern float weightBuffer[60];
extern uint8_t weightBufferReady;

void readWeight(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */