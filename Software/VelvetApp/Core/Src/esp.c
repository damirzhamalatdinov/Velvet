/* @file           : esp.c
* @date 12.03.2023  
* @author Kamalov Marat  
*/
#include "esp.h"
#include "cmsis_os2.h"
#include "app.h"
#include "adc.h"
#include "rfid.h"
#include "main.h"
#include "string.h"

typedef enum{
	ResponseError,
	RestartSTM,
	TransmitPrepareOK,
	SendWeightOK,
	TransmitWifiError,
	TimestampRespOK
} EspResponse_t;
/* Definitions for espSendQueue */
osMessageQueueId_t espSendQueueHandle;
/* Definitions for espReceiveQueue */
osMessageQueueId_t espReceiveQueueHandle;
#define RECEIVE_OK 0

static UART_HandleTypeDef* pUart;
static const uint8_t checkFWRspOK[8] = {0x01, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t getTimestampCmd[8] = {0x06, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t sendWeightPrepareCmd[8] = {0x04, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t sendWeightPrepareOK[8] = {0x04, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t sendWeightRspOK[8] = {0x05, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t sendWeightWifiErr[8] = {0x05, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t getTimestampOK[1] = {0x06};
static uint8_t checkFWCmd[8] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};// mytest
static uint8_t sendBuffer[251];
static uint8_t receiveBuffer[8] = {0};
static union weight{
	float floatVal;
	uint32_t uintVal;
} weight;
static uint16_t versionNum = 1;
static uint8_t currentCmdESP = 0;
static EspMsg_t sendMessageType = EMPTY_MSG;

EspResponse_t checkResponse(uint8_t* buf){
	if(memcmp(buf, checkFWRspOK, 8) == 0) return RestartSTM;
	else if(memcmp(buf, sendWeightPrepareOK, 8) == 0) return TransmitPrepareOK;
	else if(memcmp(buf, sendWeightRspOK, 8) == 0) return SendWeightOK;
	else if(memcmp(buf, sendWeightWifiErr, 8) == 0) return TransmitWifiError;	
	else if(memcmp(buf, getTimestampOK, 1) == 0) return TimestampRespOK;
	return ResponseError;
}

void prepareSendBuffer(void){
	uint8_t i;
	uint8_t tagBuf[6];
	
	sendBuffer[0] = 0x05;
	getCurrentTag(tagBuf);
	memcpy(&sendBuffer[1],tagBuf,6);	
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
	static EspMsg_t msgType = EMPTY_MSG;
	
	if(currentCmdESP==buf[0]){
		switch(checkResponse(buf)){
			case RestartSTM:
				HAL_NVIC_SystemReset(); //Jump to bootloader 
			break;
			case TransmitPrepareOK:
				msgType = SEND_WEIGHT;				
				osMessageQueuePut(espSendQueueHandle, &msgType, 0, 0);	
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
			case ResponseError:
			break;
		}			
	}
}

void sendMsgToESP(EspMsg_t sendMessageType){
	switch(sendMessageType){
			case WEIGHT_BUFFER_READY:
				currentCmdESP = 4;
				HAL_UART_Transmit(pUart,sendWeightPrepareCmd,8, 1000);
				//HAL_UART_Transmit_IT(&huart4,sendWeightPrepareCmd,8);		
			break;
			case CHECK_FW:
//				checkFWCmd[1] = (versionNum&0xff00)>>8;
//				checkFWCmd[2] = versionNum&0xff;
				currentCmdESP = 1;
				HAL_UART_Transmit(pUart,checkFWCmd,8,1000);				
			break;
			case TIME_SYNCHRONIZATION:
				currentCmdESP = 6;			
				HAL_UART_Transmit(pUart,getTimestampCmd,8,1000);
			break;
			case SEND_WEIGHT:
				currentCmdESP = 5;
				prepareSendBuffer();
				HAL_UART_Transmit(pUart,sendBuffer,251,1000);
			break;
			default:
			break;
	}
}

void sendMsgToESPTask(void *argument){	
	static uint8_t messageReceived = 0;
	static uint8_t ingnoreCounter = 0;
	EspMsg_t sendMessageType = EMPTY_MSG;
	
	pUart = (UART_HandleTypeDef*) argument;	
	HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_SET);
	
	for(;;)
	{			
		if(osMessageQueueGet (espSendQueueHandle, &sendMessageType, 0, MAX_DELAY) == RECEIVE_OK){
			if((currentCmdESP == 4)&&(sendMessageType!=SEND_WEIGHT)){
				ingnoreCounter++;
				if(ingnoreCounter>3) {ingnoreCounter = 0; currentCmdESP = 0;}
				continue;	//ingnore other messages, if send weight process started				
			}
			osDelay(3000);	
			HAL_UART_Receive_DMA(pUart,receiveBuffer,8);
			osDelay(200);
			sendMsgToESP(sendMessageType);			
			if(osMessageQueueGet (espReceiveQueueHandle, &messageReceived, 0, 60000) == RECEIVE_OK)
				readEspResponse(receiveBuffer);	
			else {
				HAL_UART_DMAStop(pUart);
				if((currentCmdESP == 4)||(currentCmdESP == 5)){
					currentCmdESP = 0;
					setAdcState(ADC_FREE);
				}
			}
		}		
		osDelay(1000);		
	}
}
