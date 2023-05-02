/* @file           : esp.c
* @date 12.03.2023  
* @author Kamalov Marat  
*/
#include "esp.h"
#include "cmsis_os2.h"
#include "app.h"
#include "adc.h"
#include "rfid.h"

#define transmitPrepareOK 42
#define sendWeightOK 52
#define transmitWifiError 53
#define timestampRespOK 62
#define timestampRespError 63

extern osSemaphoreId_t sendMsgSemHandle;
uint8_t checkFWCmd[8] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t getTimestampCmd[8] = {0x06, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t sendWeightPrepareCmd[8] = {0x04, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t sendWeightPrepareOK[8] = {0x04, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t sendWeightRspOK[8] = {0x05, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t sendWeightWifiErr[8] = {0x05, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t getTimestampErr[8] = {0x06, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t getTimestampOK[1] = {0x06};
uint8_t sendBuffer[251];
union weight{
	float floatVal;
	uint32_t uintVal;
} weight;
uint16_t versionNum = 1;
uint8_t currentCmdESP;
extern UART_HandleTypeDef huart4;

void sendMsgToESP(void *argument){
	for(;;)
	{
		osSemaphoreAcquire (sendMsgSemHandle, MAX_DELAY);
		if(weightBufferReady){
			HAL_UART_Transmit_IT(&huart4,sendWeightPrepareCmd,8);
			currentCmdESP = 4;			
			//adcConversionInProcess = 0;
		}
		else if(checkFW){
			checkFWCmd[1] = (versionNum&0xff00)>>8;
			checkFWCmd[2] = versionNum&0xff;
			HAL_UART_Transmit_IT(&huart4,checkFWCmd,8);
			currentCmdESP = 1;
			checkFW = 0;
		}
		else if(timeSynchronization){
			currentCmdESP = 6;
			timeSynchronization = 0;
			HAL_UART_Transmit_IT(&huart4,getTimestampCmd,8);
		}
		osDelay(1);		 
	}
}

uint8_t isBufEqual(uint8_t* buf, uint8_t* referenceBuf, uint8_t bufLen){
	uint8_t i=0;
	
	for(i=0;i<bufLen;i++){
		if(buf[i]!=referenceBuf[i]) return 0;
	}
	return 1;
}

uint8_t checkResponse(uint8_t* buf){
	if(isBufEqual(buf, sendWeightPrepareOK, 8)) return transmitPrepareOK;
	else if(isBufEqual(buf, sendWeightRspOK, 8)) return sendWeightOK;
	else if(isBufEqual(buf, sendWeightWifiErr, 8)) return transmitWifiError;
	else if(isBufEqual(buf, getTimestampErr, 8)) return timestampRespError;
	else if(isBufEqual(buf, getTimestampOK, 1)) return timestampRespOK;
	return 0;
}

void prepareSendBuffer(void){
	uint8_t i;
	
	sendBuffer[0] = 0x05;
	for(i=1;i<7;i++) sendBuffer[i] = currentTag[i-1];
	sendBuffer[7] = (uint8_t)((timestamp&0xFF000000)>>24);
	sendBuffer[8] = (uint8_t)((timestamp&0xFF0000)>>16);
	sendBuffer[9] = (uint8_t)((timestamp&0xFF00)>>8);
	sendBuffer[10] = (uint8_t)(timestamp&0xFF);
	for(i=0;i<60;i++){
		weight.floatVal = weightBuffer[i];
		sendBuffer[11+i*4+0] = (uint8_t)((weight.uintVal&0xFF000000)>>24);
		sendBuffer[11+i*4+1] = (uint8_t)((weight.uintVal&0xFF0000)>>16);
		sendBuffer[11+i*4+2] = (uint8_t)((weight.uintVal&0xFF00)>>8);
		sendBuffer[11+i*4+3] = (uint8_t) (weight.uintVal&0xFF);
	}
}

void readEspResponse(uint8_t* buf){//<<<<----- mytest testFunction
	switch(checkResponse(buf)){
		case transmitPrepareOK:
			prepareSendBuffer();
			HAL_UART_Transmit_IT(&huart4,sendBuffer,251);
			currentCmdESP = 5;
		break;
		case sendWeightOK:
			
		break;
		case transmitWifiError:
			
		break;
		case timestampRespOK:
			
		break;
		case timestampRespError:
			
		break;
	}	
	adcConversionInProcess = 0;	
}

