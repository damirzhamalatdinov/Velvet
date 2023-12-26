/** @file : temperatureSensor.c
  * @date : 18.08.2023  
  * @author Kamalov Marat  
*/
#include "main.h"
#include "temperatureSensor.h"
#include "cmsis_os2.h"

//static I2C_HandleTypeDef* pI2C;
#define TEMPERATURE_REG_SIZE 2
uint8_t receiveBuf[4];
float temperatureRFID = 0;
float temperatureExternal = 0;
uint16_t digitalOutput;
uint8_t currentSensor;

void readTemperatureValue(uint8_t devAddress){
	float temperature;
	
	digitalOutput = (receiveBuf[0]<<8|receiveBuf[1])>>4;
	if(digitalOutput&0x8000){
		temperature = ((~digitalOutput)+1)*0.0625;
	}
	else{
		temperature = digitalOutput*0.0625;
	}
	if(devAddress==RFID_TEMP_ADDR) temperatureRFID = temperature;
	if(devAddress==EXTERNAL_TEMP_ADDR) temperatureExternal = temperature;
}

/**
  * @brief  This function reads temperature register of tmp112 sensor
	* @param  I2C_HandleTypeDef - I2C handle, devAddress - device address
  * @retval None
  */
void readTemperatureRegister(I2C_HandleTypeDef* pI2C, uint8_t devAddress){
	uint8_t temperatureRegisterAddress = 0;
	uint16_t i2cCounter = 0;
	
	currentSensor = devAddress;
	HAL_I2C_Master_Transmit(pI2C, devAddress<<1, &temperatureRegisterAddress, 1, 1000);
	while (HAL_I2C_GetState(pI2C) != HAL_I2C_STATE_READY) {
		osDelay(1);
		if(i2cCounter>1000){
			HAL_I2C_Master_Abort_IT(pI2C, devAddress<<1);
			return;
		}
		else i2cCounter++;
	}
	//HAL_I2C_Master_Receive_DMA(pI2C, devAddress<<1, receiveBuf, TEMPERATURE_REG_SIZE);
	HAL_I2C_Master_Receive_IT(pI2C, devAddress<<1, receiveBuf, TEMPERATURE_REG_SIZE);
	while (HAL_I2C_GetState(pI2C) != HAL_I2C_STATE_READY) {
		osDelay(1);
		if(i2cCounter>1000){
			HAL_I2C_Master_Abort_IT(pI2C, devAddress<<1);
			return;
		}
		else i2cCounter++;
	}
	//readTemperatureValue(devAddress);	
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c){
	if (hi2c->Instance == I2C1){
		readTemperatureValue(currentSensor);
	}
}

float getTemperature(uint8_t devAddress){
	if(devAddress==EXTERNAL_TEMP_ADDR) return temperatureExternal;
	else return temperatureRFID;	
}