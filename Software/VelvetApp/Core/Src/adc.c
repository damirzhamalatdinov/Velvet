/* @file           : adc.c
* @date 19.04.2023  
* @author Kamalov Marat  
*/
#include "adc.h"
#include "hx711.h"
#include "cmsis_os2.h"
//#include "stm32f4xx_hal.h"
//#include "main.h"
#include "app.h"

extern osSemaphoreId_t readWeightSemHandle;
extern osSemaphoreId_t sendMsgSemHandle;

hx711_t loadcell;
float weightBuffer[60];
uint8_t weightIndex = 0;
uint8_t startConversion = 1;
uint8_t adcConversionInProcess = 0;
uint8_t weightBufferReady = 0;

void initADC(void)
{
	hx711_init(&loadcell, HX_SCK_GPIO_Port, HX_SCK_Pin, HX_DOUT_GPIO_Port, HX_DOUT_Pin);
  //hx711_coef_set(&loadcell, 354.5); // read afer calibration
  hx711_coef_set(&loadcell, 1.0);//no calibration, clean adc val
	osDelay(100);	
	hx711_tare(&loadcell, 10);	
	//osDelay(5000);	
	//hx711_coef_set(&loadcell, hx711_weight(&loadcell, 10)/55);//55 тарированный вес 55 г
}

void readWeight(void *argument)
{  
	initADC();	  
  for(;;)
  {
		osSemaphoreAcquire (readWeightSemHandle, MAX_DELAY);
		if(startConversion) startConversion=0;
		else {	
			adcConversionInProcess = 1;
			for(weightIndex=0;weightIndex<60;weightIndex++){
				osDelay(20);	//mytest add time management
				weightBuffer[weightIndex] = hx711_weight(&loadcell, 10);			
			}
			weightBufferReady = 1;
			osSemaphoreRelease(sendMsgSemHandle);					
		}		
  }
}