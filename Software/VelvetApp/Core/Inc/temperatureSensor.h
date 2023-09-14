/** @file : temperatureSensor.h
  * @date : 18.08.2023  
  * @author Kamalov Marat  
*/

#ifndef __TEMPERATURESENSOR_H
#define __TEMPERATURESENSOR_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "main.h"

#define RFID_TEMP_ADDR 73
#define EXTERNAL_TEMP_ADDR 72

float getTemperature(uint8_t devAddress);
void readTemperatureRegister(I2C_HandleTypeDef* pI2C, uint8_t devAddress);

#ifdef __cplusplus
}
#endif

#endif /* __TEMPERATURESENSOR_H */
