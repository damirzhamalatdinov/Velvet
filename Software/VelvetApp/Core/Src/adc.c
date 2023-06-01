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
#include "esp.h"
#include "string.h"

#define ReceiveOK 0
#define FLASH_SECTOR_6     6U

osMessageQueueId_t adcQueueHandle;
static EspMsg_t espmsg;
static hx711_t loadcell;
static float weightBuffer[60];
static uint8_t weightIndex = 0;
static uint8_t adcState = ADC_FREE;
static const uint32_t offsetAddress = 0x08040000UL;
static const uint32_t calibrationValAddress = 0x08040004UL;
static const uint32_t calibrationWeight = 10.0;
static union coefficient{
	float floatVal;
	uint32_t uintVal;	
} coefficient;


void initADC(void)
{	
	int32_t offsetUint = 0;
	int32_t offset = 0;
	uint32_t coefInt = 0;
	uint32_t ffVal = 0xFFFFFFFFUL;
	float coef = 0;
		
	offsetUint = *(__IO uint32_t*)offsetAddress;
	coefInt = *(__IO uint32_t*)calibrationValAddress;
	//memcpy(&offset, (uint32_t *)&offsetAddress, 4);
	//memcpy(&coefInt, (uint32_t *)&calibrationValAddress, 4);	
	hx711_init(&loadcell, HX_SCK_GPIO_Port, HX_SCK_Pin, HX_DOUT_GPIO_Port, HX_DOUT_Pin);
  //hx711_coef_set(&loadcell, 354.5); // read afer calibration
	if(memcmp(&coefInt,&ffVal,4) != 0){
	//if (coefInt != 0xFFFFFFFFUL){
		memcpy(&coef, &coefInt, 4);
		hx711_coef_set(&loadcell, coef);		
	}
  else hx711_coef_set(&loadcell, 1.0);//no calibration, clean adc val
	osDelay(100);	
	if(memcmp(&offsetUint,&ffVal,4) != 0){
		memcpy(&offset, &offsetUint, 4);
		hx711_offset_set(&loadcell, offset);		
	}
	//if (offset != 0xFFFFFFFFUL) hx711_offset_set(&loadcell, offset);
	else hx711_tare(&loadcell, 10);	
	//osDelay(5000);	
	//hx711_coef_set(&loadcell, hx711_weight(&loadcell, 10)/55);//55 тарированный вес 55 г
}

void saveCoefficientsToFlash(int32_t offset, float calibrationValue){
	static uint32_t SectorError = 0;
	static FLASH_EraseInitTypeDef EraseInitStruct;
	
	HAL_FLASH_Unlock();
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = FLASH_SECTOR_6;//0x08040000) /* Base @ of Sector 6, 128 Kbytes
  EraseInitStruct.NbSectors = 1;
	HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, offsetAddress, offset);
	coefficient.floatVal = calibrationValue;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, calibrationValAddress, coefficient.uintVal);
	HAL_FLASH_Lock();
}

void readWeightTask(void *argument)
{ 	
	static AdcMsg_t adcMsg;		
	
	initADC();	  
  for(;;)
  {
		if(osMessageQueueGet (adcQueueHandle, &adcMsg, 0, MAX_DELAY) == ReceiveOK){		
			if(adcMsg == ReadWeight){
				adcState = ADC_BUSY;
				for(weightIndex=0;weightIndex<60;weightIndex++){
					osDelay(20);	//mytest add time management
					weightBuffer[weightIndex] = hx711_weight(&loadcell, 10);			
				}	
				//adcState = ADC_FREE;// mytest			
				espmsg = WeightBufferReady;
				osMessageQueuePut(espSendQueueHandle, &espmsg, 0, 0);	
			}
			else if (adcMsg == Calibration){				
				hx711_calibration(&loadcell, hx711_offset_get(&loadcell), hx711_value_ave(&loadcell, 10), calibrationWeight);
				saveCoefficientsToFlash(hx711_offset_get(&loadcell), hx711_coef_get(&loadcell));
			}
			else if (adcMsg == SetOffset){
				hx711_tare(&loadcell, 10);
				saveCoefficientsToFlash(hx711_offset_get(&loadcell), hx711_coef_get(&loadcell));
			}
		}		
  }
}

uint8_t getAdcState(void){
	return adcState;
}

void setAdcState(uint8_t state){
	adcState = state;
}

float getWeightValByIndex (uint8_t index){
	return weightBuffer[index];
}