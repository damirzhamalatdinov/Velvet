/* @file           : adc.c
 * @date 19.04.2023
 * @author Kamalov Marat
 */
#include "adc.h"
#include "hx711.h"
#include "cmsis_os2.h"
// #include "stm32f4xx_hal.h"
// #include "main.h"
#include "app.h"
#include "esp.h"
#include "string.h"

//#define HX711
#define AD7797
#define RECEIVE_OK 0
#define SAMPLE_NUMBER 10
#define FLASH_SECTOR_6     6U
#define READ_ID_REGISTER 96 //0b01100000
#define READ_MODE_REGISTER 72 //0b01001000
#define READ_CONFIGURATION_REGISTER 80 //0b01010000
#define READ_DATA_REGISTER 88 //0b01011000
#define READ_STATUS_REGISTER 64 //0b01000000
#define RESET_ADC 0xff
#define ADC_ERROR					   -1
#define ADC_OK						    0

extern SPI_HandleTypeDef hspi2;
osMessageQueueId_t adcQueueHandle;
static EspMsg_t espmsg;
static hx711_t loadcell;
static float weightBuffer[60];
static uint8_t weightIndex = 0;
static uint8_t adcState = ADC_FREE;
static const uint32_t offsetAddress = 0x08040000UL;
static const uint32_t calibrationValAddress = 0x08040004UL;
static const uint32_t calibrationWeight = 10.0;
uint8_t communicationRegister[8] = {96,0x00,96,0x00,96,0x00,96,0x00};//0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t adcBuffer[8]={0};
int8_t adcInitState = ADC_ERROR;
static union coefficient{
	float floatVal;
	uint32_t uintVal;	
} coefficient;

void initHX711(void){
#ifdef HX711	
  int32_t offsetUint = 0;
	int32_t offset = 0;
	uint32_t coefInt = 0;
	uint32_t ffVal = 0xFFFFFFFFUL;
	float coef = 0;
		
	offsetUint = *(__IO uint32_t*)offsetAddress;
	coefInt = *(__IO uint32_t*)calibrationValAddress;	
	hx711_init(&loadcell, HX_SCK_GPIO_Port, HX_SCK_Pin, HX_DOUT_GPIO_Port, HX_DOUT_Pin);
  hx711_coef_set(&loadcell, 354.5); // read afer calibration
	if(memcmp(&coefInt,&ffVal,4) != 0){
	//if (coefInt != 0xFFFFFFFFUL){
		memcpy(&coef, &coefInt, 4);
		hx711_coef_set(&loadcell, coef);		
	}
  else hx711_coef_set(&loadcell, 1.0);//no calibration, clean adc val
	osDelay(100);	
	if(memcmp(&offsetUint,&ffVal,4) != 0){
		memcpy(&offset, &offsetUint, 4);
		//hx711_offset_set(&loadcell, offset);		
	}
	//if (offset != 0xFFFFFFFFUL) hx711_offset_set(&loadcell, offset);
	else hx711_tare(&loadcell, SAMPLE_NUMBER);	
	//osDelay(5000);	
	//hx711_coef_set(&loadcell, hx711_weight(&loadcell, 10)/55);//55 ������������ ��� 55 �
	adcInitState = ADC_OK;
#endif
}

HAL_StatusTypeDef adcError;

void resetADC(uint8_t* readBuffer){
	uint8_t commandBuf[4]={0xff,0xff,0xff,0xff};	
	
	HAL_SPI_TransmitReceive(&hspi2, commandBuf, readBuffer, 4, 5000);
}

void getADCRegister(uint8_t command, uint8_t* readBuffer, uint8_t registerSize){
	uint8_t i;
	uint8_t commandBuf[2]={0, 0};
	
	commandBuf[0]=command;
	HAL_SPI_TransmitReceive(&hspi2, commandBuf, readBuffer, 1, 5000);
	for(i=1;i<registerSize+1;i++) HAL_SPI_TransmitReceive(&hspi2, &commandBuf[1], &readBuffer[i], 1, 5000);
}

uint16_t conversionCounter = 0;
uint16_t delayCounter = 0;
void initAD7797(void){		
	HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_RESET);
	resetADC(adcBuffer);	
	getADCRegister(READ_ID_REGISTER, adcBuffer, 1);
	if(adcBuffer[1]==0x5B) adcInitState = ADC_OK;
	else adcInitState = ADC_ERROR;
	osDelay(100);
	for(;;){
		if(HAL_GPIO_ReadPin(ADC_DOUT_GPIO_Port, ADC_DOUT_Pin) == GPIO_PIN_RESET){
			getADCRegister(READ_STATUS_REGISTER, adcBuffer, 1);	
			conversionCounter++;
			if((adcBuffer[1]&0x80)==0){
				getADCRegister(READ_DATA_REGISTER, adcBuffer, 3);	
				osDelay(100);
			}
		}
		else{
			osDelay(1);
			delayCounter++;
		}
	}
  HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_SET); 
}

void initADC(void)
{	
	int32_t offsetUint = 0;
	int32_t offset = 0;
	uint32_t coefInt = 0;
	uint32_t ffVal = 0xFFFFFFFFUL;
	float coef = 0;
		
	offsetUint = *(__IO uint32_t*)offsetAddress;
	coefInt = *(__IO uint32_t*)calibrationValAddress;

#ifdef HX711
	initHX711();
#endif	
#ifdef AD7797
	initAD7797();
#endif
}

float readWeightAD7797(uint32_t offset, float coefficient){
	float weight;
	uint32_t adcValue;
	
	for(uint8_t i=0; i<3;i++){
		HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_RESET);
		communicationRegister[0] = READ_STATUS_REGISTER;
		HAL_SPI_TransmitReceive(&hspi2, communicationRegister, adcBuffer, 2, 1000);
		if((adcBuffer[0]&0x80)==0){
			communicationRegister[0] = READ_DATA_REGISTER;
			HAL_SPI_Transmit(&hspi2, communicationRegister, 2, 1000);
			HAL_SPI_Receive(&hspi2, adcBuffer, 3, 1000);
			HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_SET); 
			adcValue = adcBuffer[0]<<16;
			adcValue |= adcBuffer[1]<<8;
			adcValue |= adcBuffer[2];
			weight = (adcValue - offset)/coefficient;
			return weight;
		}
		else {
			HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_SET); 
			osDelay(250);		
		}
	}
	return 0;
}

float readWeight(void){
#ifdef HX711
	return hx711_weight(&loadcell, 10);	
#endif	
#ifdef AD7797
	return readWeightAD7797(0, 1);
#endif
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
	uint8_t initCounter = 0;	
	
	initADC();
	while (adcInitState != ADC_OK)
	{
			osDelay(1000); // mytest add time management
			initCounter++;
			if (initCounter >= 255)
			{
					initCounter = 0;
					initADC();
			}
	}	
	for(;;)
	{
		if(osMessageQueueGet (adcQueueHandle, &adcMsg, 0, MAX_DELAY) == RECEIVE_OK){		
			if(adcMsg == READ_WEIGHT){
				adcState = ADC_BUSY;
				for(weightIndex=0;weightIndex<60;weightIndex++){
					osDelay(20);	//mytest add time management
					weightBuffer[weightIndex] = readWeight();
				}	
				//adcState = ADC_FREE;// mytest			
				espmsg = WEIGHT_BUFFER_READY;
				osMessageQueuePut(espSendQueueHandle, &espmsg, 0, 0);	
			}
			else if (adcMsg == CALIBRATION){				
				//hx711_calibration(&loadcell, hx711_offset_get(&loadcell), hx711_value_ave(&loadcell, 10), calibrationWeight);
				//saveCoefficientsToFlash(hx711_offset_get(&loadcell), hx711_coef_get(&loadcell));
			}
			else if (adcMsg == SET_OFFSET){
				hx711_tare(&loadcell, SAMPLE_NUMBER);
				//saveCoefficientsToFlash(hx711_offset_get(&loadcell), hx711_coef_get(&loadcell));
			}
		}		
	}
}

uint8_t getAdcState(void)
{
	return adcState;
}

void setAdcState(uint8_t state)
{
	adcState = state;
}

float getWeightValByIndex(uint8_t index)
{
	return weightBuffer[index];
}