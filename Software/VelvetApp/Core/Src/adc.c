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
osMessageQueueId_t spiQueueHandle;
static SPI_HandleTypeDef* pAdcSpi;
static EspMsg_t espmsg;
static hx711_t loadcell;
static float weightBuffer[60];
static uint8_t weightIndex = 0;
static uint8_t adcState = ADC_FREE;
static const uint32_t offsetAddress = 0x08040000UL;
static const uint32_t calibrationValAddress = 0x08040004UL;
static const uint32_t calibrationWeight = 10.0;
static uint32_t adcOffset;
static float adcCoefficient;
uint8_t communicationRegister[8] = {96,0x00,96,0x00,96,0x00,96,0x00};//0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t adcBuffer[8]={0};
int8_t adcInitState = ADC_ERROR;
static union floatUintConv{
	float floatVal;
	uint32_t uintVal;	
} floatUintConv;

float readWeightAD7797(uint32_t offset, float coefficient);

void initHX711(int32_t offset, float coef){
#ifdef HX711	
  hx711_init(&loadcell, HX_SCK_GPIO_Port, HX_SCK_Pin, HX_DOUT_GPIO_Port, HX_DOUT_Pin);
  hx711_coef_set(&loadcell, 354.5); // read afer calibration
	hx711_coef_set(&loadcell, coef);
	osDelay(100);
	hx711_offset_set(&loadcell, offset);	
	//hx711_tare(&loadcell, SAMPLE_NUMBER);	
	//osDelay(5000);	
	//hx711_coef_set(&loadcell, hx711_weight(&loadcell, 10)/55);//55 ������������ ��� 55 �
	adcInitState = ADC_OK;
#endif
}

void resetADC(uint8_t* readBuffer){
	uint8_t commandBuf[4]={0xff,0xff,0xff,0xff};	
	
	HAL_SPI_TransmitReceive(&hspi2, commandBuf, readBuffer, 4, 5000);
}

int8_t getADCRegister(uint8_t command, uint8_t* readBuffer, uint8_t registerSize){
	uint8_t i;
	uint8_t commandBuf[4]={0};
	uint8_t spiMsg = 1;
	
	commandBuf[0]=command;
	if(command == 0xff) {
		for(i=1;i<4;i++) commandBuf[i]=0xff;
	}
	HAL_SPI_TransmitReceive_DMA(pAdcSpi, commandBuf, readBuffer, registerSize);//, 5000);
	if(osMessageQueueGet (spiQueueHandle, &spiMsg, 0, MAX_DELAY) == RECEIVE_OK){
		if(spiMsg == RECEIVE_OK) return ADC_OK;
	}
	return ADC_ERROR;
	//for(i=1;i<registerSize+1;i++) HAL_SPI_TransmitReceive(&hspi2, &commandBuf[1], &readBuffer[i], 1, 5000);
}

uint16_t conversionCounter = 0;
uint16_t delayCounter = 0;
void initAD7797(void){	
	uint32_t adcValue;	
	
	HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_RESET);
	//resetADC(adcBuffer);	
	getADCRegister(RESET_ADC, adcBuffer, 4);
	getADCRegister(READ_ID_REGISTER, adcBuffer, 2);
	if(adcBuffer[1]==0x5B) adcInitState = ADC_OK;
	else adcInitState = ADC_ERROR;
	//readWeightAD7797(adcOffset, 1);//adcCoefficient);	
	HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_SET);	
	
//	osDelay(100);
//	for(;;){
//		if(HAL_GPIO_ReadPin(ADC_DOUT_GPIO_Port, ADC_DOUT_Pin) == GPIO_PIN_RESET){
//			getADCRegister(READ_STATUS_REGISTER, adcBuffer, 1);	
//			conversionCounter++;
//			if((adcBuffer[1]&0x80)==0){
//				getADCRegister(READ_DATA_REGISTER, adcBuffer, 3);	
//				osDelay(100);				
//			}
//		}
//		else{
//			osDelay(1);
//			delayCounter++;
//		}
//	}   
}

void initADC(void)
{	
	uint32_t offsetUint = 0;
	uint32_t offset = 0;
	uint32_t coefInt = 0;
	uint32_t ffVal = 0xFFFFFFFFUL;
	float coef = 1.0;
		
	offsetUint = *(__IO uint32_t*)offsetAddress;
	coefInt = *(__IO uint32_t*)calibrationValAddress;
	if(memcmp(&offsetUint,&ffVal,4) != 0){
		memcpy(&offset, &offsetUint, 4);				
	}
	if(memcmp(&coefInt,&ffVal,4) != 0){	
		memcpy(&coef, &coefInt, 4);				
	}  
#ifdef HX711
	initHX711(offset, coef);
#endif	
#ifdef AD7797
	adcOffset = offset;
	adcCoefficient = coef;
	initAD7797();
#endif
}

float weight;
uint8_t errorFlag = 0;
float readWeightAD7797(uint32_t offset, float coeff){	
	uint32_t adcValue;	
	
	for(uint8_t i=0; i<5;i++){		
		if(HAL_GPIO_ReadPin(ADC_DOUT_GPIO_Port, ADC_DOUT_Pin) == GPIO_PIN_RESET){			
			getADCRegister(READ_STATUS_REGISTER, adcBuffer, 2);	
			conversionCounter++;
			if((adcBuffer[1]&0x80)==0){
				if((adcBuffer[1]&0x40)) errorFlag = 1;
				else errorFlag = 0;
				getADCRegister(READ_DATA_REGISTER, adcBuffer, 4);	
				adcValue = adcBuffer[1]<<16;
				adcValue |= adcBuffer[2]<<8;
				adcValue |= adcBuffer[3];
				if(errorFlag == 0) weight = ((float)offset-adcValue)/coeff;
				else weight = 0;
				return weight;				
				//break;
			}
		}
		else{
			osDelay(50);
			delayCounter++;
		}				
	}	
	return 0;
}

float readWeight(uint32_t offset, float coefficient){
#ifdef HX711
	return hx711_weight(&loadcell, 10);	
#endif	
#ifdef AD7797
	return readWeightAD7797(offset, coefficient);
#endif
}

void saveCoefficientsToFlash(uint32_t offset, float calibrationValue){
	static uint32_t SectorError = 0;
	static FLASH_EraseInitTypeDef EraseInitStruct;
	
	HAL_FLASH_Unlock();
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = FLASH_SECTOR_6;//0x08040000) /* Base @ of Sector 6, 128 Kbytes
  EraseInitStruct.NbSectors = 1;
	HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, offsetAddress, offset);
	floatUintConv.floatVal = calibrationValue;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, calibrationValAddress, floatUintConv.uintVal);
	HAL_FLASH_Lock();
}

void runADC(void){
	#ifdef AD7797
	HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_RESET);
	osDelay(70);
	#endif
}

void stopADC(void){
	#ifdef AD7797
	HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_SET);
	#endif
}

float findAverage(float *a, uint8_t n){
	float avg = 0;
	for (uint16_t i = 0; i < n; i++){  
		avg+=a[i];
	}
	return avg/n;
}

void startCalibration(){	
	#ifdef HX711
		hx711_calibration(&loadcell, hx711_offset_get(&loadcell), hx711_value_ave(&loadcell, 10), calibrationWeight);
		saveCoefficientsToFlash(hx711_offset_get(&loadcell), hx711_coef_get(&loadcell));
	#endif	
	#ifdef AD7797
		adcState = ADC_BUSY;
		runADC();
		for(weightIndex=0;weightIndex<10;weightIndex++){					
			weightBuffer[weightIndex] = readWeight(adcOffset, 1);
			osDelay(15);
		}	
		stopADC();		
		adcCoefficient = calibrationWeight/findAverage(weightBuffer, 10);	
		adcState = ADC_FREE;
		saveCoefficientsToFlash(adcOffset, adcCoefficient);
	#endif
}

void setOffset(void){
	#ifdef HX711
		hx711_tare(&loadcell, SAMPLE_NUMBER);
		saveCoefficientsToFlash(hx711_offset_get(&loadcell), hx711_coef_get(&loadcell));
	#endif	
	#ifdef AD7797
		adcState = ADC_BUSY;
		runADC();
		for(weightIndex=0;weightIndex<10;weightIndex++){					
			weightBuffer[weightIndex] = readWeight(0, 1);
			osDelay(15);
		}	
		stopADC();		
		adcOffset = (uint32_t)(findAverage(weightBuffer, 10)*(-1));	
		adcState = ADC_FREE;
		saveCoefficientsToFlash(adcOffset, adcCoefficient);
	#endif
}

void readWeightTask(void *argument)
{ 	
	static AdcMsg_t adcMsg;	
	uint8_t initCounter = 0;	
	
	pAdcSpi = (SPI_HandleTypeDef*) argument;	
	initADC();
	while (adcInitState != ADC_OK)
	{
			osDelay(100); // mytest add time management
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
				runADC();
				for(weightIndex=0;weightIndex<60;weightIndex++){					
					weightBuffer[weightIndex] = readWeight(adcOffset, adcCoefficient);
					osDelay(15);
				}	
				stopADC();
				//adcState = ADC_FREE;// mytest			
				espmsg = WEIGHT_BUFFER_READY;
				osMessageQueuePut(espSendQueueHandle, &espmsg, 0, 0);	
			}
			else if (adcMsg == CALIBRATION) startCalibration();				
			else if (adcMsg == SET_OFFSET) setOffset();
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

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
	if (hspi == pAdcSpi){
		uint8_t spiMsg = RECEIVE_OK;
		
		osMessageQueuePut(spiQueueHandle, &spiMsg, 0, 0);
	}
	
}