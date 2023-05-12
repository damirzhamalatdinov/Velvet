/* @file           : esp.c
* @date 12.03.2023  
* @author Kamalov Marat  
*/
#include "esp.h"
#include "cmsis_os2.h"
#include "app.h"
#include "adc.h"
#include "rfid.h"

#define RestartSTM 12
#define TransmitPrepareOK 42
#define SendWeightOK 52
#define TransmitWifiError 53
#define TimestampRespOK 62
#define ReceiveOK 1

static const uint8_t checkFWRspOK[8] = {0x01, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t getTimestampCmd[8] = {0x06, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t sendWeightPrepareCmd[8] = {0x04, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t sendWeightPrepareOK[8] = {0x04, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t sendWeightRspOK[8] = {0x05, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t sendWeightWifiErr[8] = {0x05, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t getTimestampOK[1] = {0x06};
static uint8_t checkFWCmd[8] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t sendBuffer[251];
static uint8_t receiveBuffer[8] = {0};
static union weight{
	float floatVal;
	uint32_t uintVal;
} weight;
static uint16_t versionNum = 1;
static uint8_t currentCmdESP = 0;
static EspMsg_t sendMessageType = None;

extern UART_HandleTypeDef huart4;
extern osMessageQueueId_t espSendQueueHandle;
extern osMessageQueueId_t espReceiveQueueHandle;

uint8_t isBufEqual(uint8_t* buf, const uint8_t* referenceBuf, uint8_t bufLen){
	uint8_t i=0;
	
	for(i=0;i<bufLen;i++){
		if(buf[i]!=referenceBuf[i]) return 0;
	}
	return 1;
}

uint8_t checkResponse(uint8_t* buf){
	if(isBufEqual(buf, checkFWRspOK, 8)) return RestartSTM;
	else if(isBufEqual(buf, sendWeightPrepareOK, 8)) return TransmitPrepareOK;
	else if(isBufEqual(buf, sendWeightRspOK, 8)) return SendWeightOK;
	else if(isBufEqual(buf, sendWeightWifiErr, 8)) return TransmitWifiError;	
	else if(isBufEqual(buf, getTimestampOK, 1)) return TimestampRespOK;
	return 0;
}

void prepareSendBuffer(void){
	uint8_t i;
	uint8_t tagBuf[6];
	
	sendBuffer[0] = 0x05;
	getCurrentTag(tagBuf);
	for(i=1;i<7;i++) sendBuffer[i] = tagBuf[i-1];
	sendBuffer[7] = (uint8_t)((timestamp&0xFF000000)>>24);
	sendBuffer[8] = (uint8_t)((timestamp&0xFF0000)>>16);
	sendBuffer[9] = (uint8_t)((timestamp&0xFF00)>>8);
	sendBuffer[10] = (uint8_t)(timestamp&0xFF);
	for(i=0;i<60;i++){
		weight.floatVal = getWeightValByIndex(i);
		sendBuffer[11+i*4+0] = (uint8_t)((weight.uintVal&0xFF000000)>>24);
		sendBuffer[11+i*4+1] = (uint8_t)((weight.uintVal&0xFF0000)>>16);
		sendBuffer[11+i*4+2] = (uint8_t)((weight.uintVal&0xFF00)>>8);
		sendBuffer[11+i*4+3] = (uint8_t) (weight.uintVal&0xFF);
	}
}

void readEspResponse(uint8_t* buf){//<<<<----- mytest testFunction
	static uint32_t timest;
	
	if(currentCmdESP==buf[0]){
		switch(checkResponse(buf)){
			case RestartSTM:
				HAL_NVIC_SystemReset(); //Jump to bootloader 
			break;
			case TransmitPrepareOK:
				sendMessageType = SendWeight;				
				osMessageQueuePut(espSendQueueHandle, &sendMessageType, 0, 0);	
			break;
			case SendWeightOK:
				setAdcState(ADC_FREE);				
			break;
			case TransmitWifiError:
				//mytest add send LoRa or GSM functionality
			break;
			case TimestampRespOK:
				timest = buf[1];
			  timest = (timest<<8)|buf[2];
				timest = (timest<<8)|buf[3];
			  timest = (timest<<8)|buf[4];
				timestamp = timest;				
			break;			
		}			
	}
}

void sendMsgToESP(void){
	switch(sendMessageType){
			case WeightBufferReady:
				currentCmdESP = 4;
				HAL_UART_Transmit(&huart4,sendWeightPrepareCmd,8, 1000);
				//HAL_UART_Transmit_IT(&huart4,sendWeightPrepareCmd,8);		
			break;
			case CheckFW:
				checkFWCmd[1] = (versionNum&0xff00)>>8;
				checkFWCmd[2] = versionNum&0xff;
				currentCmdESP = 1;
				HAL_UART_Transmit(&huart4,checkFWCmd,8,1000);				
			break;
			case TimeSynchronization:
				currentCmdESP = 6;			
				HAL_UART_Transmit(&huart4,getTimestampCmd,8,1000);
			break;
			case SendWeight:
				currentCmdESP = 5;
				prepareSendBuffer();
				HAL_UART_Transmit(&huart4,sendBuffer,251,1000);
			break;
			default:
			break;
	}
}

void sendMsgToESPTask(void *argument){	
	static uint8_t messageReceived = 0;
	static uint8_t ingnoreCounter = 0;
	
	for(;;)
	{		
		osMessageQueueGet (espSendQueueHandle, &sendMessageType, 0, MAX_DELAY);
		if((currentCmdESP == 4)&&(sendMessageType!=SendWeight)){
			ingnoreCounter++;
			if(ingnoreCounter>3) {ingnoreCounter = 0; currentCmdESP = 0;}
			continue;	//ingnore other messages, if send weight process started				
		}
		sendMsgToESP();
		HAL_UART_Receive_DMA(&huart4,receiveBuffer,8);
		if(osMessageQueueGet (espReceiveQueueHandle, &messageReceived, 0, 1000) == ReceiveOK)
			readEspResponse(receiveBuffer);		
		osDelay(1);		 		
	}
}
