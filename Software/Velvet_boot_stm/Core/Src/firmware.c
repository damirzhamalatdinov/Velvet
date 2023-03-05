#include "firmware.h"
#include <math.h>

extern UART_HandleTypeDef huart4;

uint8_t bootloaderReadyCmd[8] = {0x02, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t receiveReadyCmd[8]    = {0x03, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t receiveErrorRsp[8]    = {0x03, 0xFF, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t uartReadBuf[258];
//uint16_t firmwareSize = 0;
uint8_t sum2 = 0;

bool isEspReady(uint16_t* firmwareSize){
	uint16_t availableFlashMemory = 0;
	
	if(HAL_UART_Transmit(&huart4, bootloaderReadyCmd, 8, 5000) == HAL_OK){// 0xFFFF 65,5s timeout
		if(HAL_UART_Receive(&huart4, uartReadBuf, 8, 15000) == HAL_OK)// 5s timeout
			if(uartReadBuf[0] == 2 && (uartReadBuf[1] != 0 || uartReadBuf[2] != 0)){
				*firmwareSize = (uartReadBuf[1] << 8) | uartReadBuf[2];//number of 256 bytes pages
				availableFlashMemory = (LAST_SECTOR_NUM - APP_START_SECTOR)*128;//kBytes
				if ((((float)*firmwareSize*256)/1024)<=availableFlashMemory) return true;
			}
	}
  return false;
}

void eraseFirmwareSectors(uint16_t firmwareSize){
	uint8_t firmwareSectorsNumber;
	
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
	firmwareSectorsNumber = (uint8_t)ceilf(((float)firmwareSize*256)/1024/128);
	for(uint8_t i=0;i<firmwareSectorsNumber;i++){
		FLASH_Erase_Sector(FLASH_SECTOR_5+i, VOLTAGE_RANGE_3);
	}
}

bool isValidData(uint8_t *data, uint16_t size){
  uint8_t sum = 0;
   
  for (uint16_t i = 0;i<size-1;i++){
    sum += data[i];
  }
  if (sum == data[size-1]) return true;    
  return false;
}

void receiveFirmware (uint8_t* status, uint32_t* address){
	uint32_t data;
	
	if(HAL_UART_Receive(&huart4, uartReadBuf, 258, 500) == HAL_OK){
		if((uartReadBuf[0] == 3)&&isValidData(uartReadBuf, 258))
			for(uint16_t j=0;j<256;j+=4,*address+=4){						
					data = uartReadBuf[j+4] << 24;
					data |= uartReadBuf[j+3] << 16;
					data |= uartReadBuf[j+2] << 8;
					data |= uartReadBuf[j+1];
					HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, *address, data);
			}
		else *status=1;
	}
}

void loadFirmware(uint16_t firmwareSize){
	uint8_t status = 0;	
	uint32_t address = APP_START_ADDR;
	
	uint32_t tickstart = HAL_GetTick();  
	for(uint8_t i=0;i<firmwareSize;){
		if((HAL_GetTick()-tickstart)<60000) tickstart = HAL_GetTick();
		else return;
		if(status == 0) {
			if(HAL_UART_Transmit(&huart4, receiveReadyCmd, 8, 500) == HAL_OK) 
				receiveFirmware (&status, &address);
		}
		else {
			status=0;			
			if(HAL_UART_Transmit(&huart4, receiveErrorRsp, 8, 500) == HAL_OK)
				receiveFirmware (&status, &address);
		}
		if(status == 0) i++;
	}
}

void updateFirmware(void){	
	uint16_t firmwareSize = 0;
	
	if(isEspReady(&firmwareSize) == true){
		HAL_FLASH_Unlock();
		eraseFirmwareSectors(firmwareSize);
		loadFirmware(firmwareSize);
		HAL_FLASH_Lock();
	}
}
