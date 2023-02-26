#include "firmware.h"

extern UART_HandleTypeDef huart4;

uint8_t getUpdateStatusCmd[3] = {0x02, 0x01, 0x03};
uint8_t bootloaderReadyCmd[8] = {0x02, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t UpdateCmd[3] = {0x02, 0x02, 0x04};
uint8_t DataOK[3] = {0x02, 0x03, 0x05};
uint8_t uartReadBuf[257];
uint16_t firmwareSize = 0;
uint8_t sum2 = 0;

void updateFirmware(void){	
	if(isUpdateReady() == true){
	sum2++;
	}
}

bool isValidData(uint8_t *data, size_t size){
  uint8_t sum = 0;
  int i = 0;
  for (;i < size-1;i++){
    sum += data[i];
  }
  if (sum  == data[size - 1])
    return true;
  return false;
}

bool isUpdateReady(void){
	uint16_t availableFlashMemory = 0;
	
	if(HAL_UART_Transmit(&huart4, bootloaderReadyCmd, 8, 5000) == HAL_OK){// 0xFFFF 65,5s timeout
		if(HAL_UART_Receive(&huart4, uartReadBuf, 8, 15000) == HAL_OK)// 5s timeout
			if(uartReadBuf[0] == 2 && (uartReadBuf[1] != 0 || uartReadBuf[2] != 0)){
				firmwareSize = ((uartReadBuf[1] << 8) | uartReadBuf[2])*256/1024;//kBytes
				if (APP_START_SECTOR == FLASH_SECTOR_4) availableFlashMemory = 64 + (LAST_SECTOR_NUM - APP_START_SECTOR)*128;//kBytes
				if (firmwareSize<=availableFlashMemory){
					firmwareSize = ((uartReadBuf[1] << 8) | uartReadBuf[2]);//number of 256 bytes pages
					return true;				
				}
			}
	}
  return false;
}


bool updateFw(UART_HandleTypeDef *huart){
    // USBPrint("--- UPDATE START ---\n");
	uint32_t internAddr = APP_START_ADDR;
	uint8_t buf[5] = { 0 };
	uint8_t sizeBuf[5] = { 0 };
	uint32_t data = 0;
	uint16_t appSize = -1;
	uint8_t out[20];

  HAL_UART_Transmit(huart, UpdateCmd, 3, 0xFFFF);
	HAL_UART_Receive(huart, sizeBuf, sizeof(sizeBuf), 10000);
	if (isValidData(sizeBuf, sizeof(sizeBuf))){
		
		appSize = ((sizeBuf[1] << 16) | (sizeBuf[2] << 8)) | sizeBuf[3];
	} else {
		return false;
	}
	HAL_StatusTypeDef flashStatus;
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
    FLASH_Erase_Sector(FLASH_SECTOR_4, VOLTAGE_RANGE_3);
	FLASH_Erase_Sector(FLASH_SECTOR_5, VOLTAGE_RANGE_3);
    // eraseAppInMpu();

	uint8_t stmCheckSum = 0, espCheckSum = -1;
	int i;
	uint8_t dataOk[1] = {0x74};
	for (i = 0; i < appSize; i += 4) {
		HAL_UART_Transmit(huart, dataOk, sizeof(dataOk), 0xFFFF);
		HAL_UART_Receive(huart, buf, 5, 10000);
		uint8_t tempSum = buf[0] + buf[1] + buf[2] + buf[3];
		if ( tempSum != buf[4]){
			return false;
		}
		data = buf[3] << 24;
		data |= buf[2] << 16;
		data |= buf[1] << 8;
		data |= buf[0];

		flashStatus = HAL_FLASH_Program(
		FLASH_TYPEPROGRAM_WORD, internAddr, data);
		internAddr += 4;
        
		if (flashStatus != HAL_OK) {
            // USBPrint("UPDATE false");
			return false;
		}
		if(appSize - i <= 16){
			// USBprintArray(buf, 4);
		}
	}
	
    HAL_FLASH_Lock();

	return true;
}