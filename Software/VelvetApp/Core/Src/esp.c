/** @file esp.c
	* @brief Исходный код функций для отправки и приёма сообщений от микроконтроллера ESP
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
/// Набор возможных ответов микроконтроллера ESP
typedef enum{
	RESPONSE_ERROR,
	RESTART_STM,
	TRANSMIT_PREPARE_OK,
	SEND_WEIGHT_OK,
	TRANSMIT_WIFI_ERROR,
	TIMESTAMP_RESPONSE_OK
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
static uint8_t checkFWCmd[8] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t sendBuffer[251];
static uint8_t receiveBuffer[8] = {0};
static union weight{
	float floatVal;
	uint32_t uintVal;
} weight;
static uint16_t versionNum = 1;
static uint8_t currentCmdESP = 0;
/**
	* @brief Функция проверки ответа от микроконтроллера ESP	
  * @param buf Массив с ответом микроконтроллера ESP
  * @retval Код ответа микроконтроллера ESP
  */
EspResponse_t checkResponse(uint8_t* buf){
	if(memcmp(buf, checkFWRspOK, 8) == 0) return RESTART_STM;
	else if(memcmp(buf, sendWeightPrepareOK, 8) == 0) return TRANSMIT_PREPARE_OK;
	else if(memcmp(buf, sendWeightRspOK, 8) == 0) return SEND_WEIGHT_OK;
	else if(memcmp(buf, sendWeightWifiErr, 8) == 0) return TRANSMIT_WIFI_ERROR;	
	else if(memcmp(buf, getTimestampOK, 1) == 0) return TIMESTAMP_RESPONSE_OK;
	return RESPONSE_ERROR;
}
/**
	* @brief Функция подготовки буфера измеренных весов для отправки в микроконтроллер ESP	
	* @details Также отправляется RFID метка животного и метка времени
  * @param Отсутствует
  * @retval Отсутствует
  */
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
/**
	* @brief Функция чтения ответа от микроконтроллера ESP	
  * @details В соответствии с ответом ESP запускаются функции:
	- Перезагрузка микроконтроллера
	- Отправка массива с измеренными весами
	- Разблокировка АЦП
	- Обновление системного времени
  * @param buf Массив с ответом микроконтроллера ESP
  * @retval Отсутствует
  */
void readEspResponse(uint8_t* buf){//<<<<----- mytest testFunction
	static uint32_t timest;
	static EspMsg_t msgType = EMPTY_MSG;
	
	if(currentCmdESP==buf[0]){
		switch(checkResponse(buf)){
			case RESTART_STM:
				HAL_NVIC_SystemReset(); //Jump to bootloader 
			break;
			case TRANSMIT_PREPARE_OK:
				msgType = SEND_WEIGHT;				
				osMessageQueuePut(espSendQueueHandle, &msgType, 0, 0);	
			break;
			case SEND_WEIGHT_OK:
				setAdcState(ADC_FREE);				
			break;
			case TRANSMIT_WIFI_ERROR:
				//mytest add send LoRa or GSM functionality
			break;
			case TIMESTAMP_RESPONSE_OK:
				timest = buf[1];
			  timest = (timest<<8)|buf[2];
				timest = (timest<<8)|buf[3];
			  timest = (timest<<8)|buf[4];
				timestamp = timest;				
			break;	
			case RESPONSE_ERROR:
			break;
		}			
	}
}
/**
	* @brief Функция отправки сообщения микроконтроллеру ESP	
  * @details В соответствии типом сообщения производится отправка соответствующей последовательности байт:
	- Подготовка ESP к приёму массива весов
	- Проверка наличия обновлений ПО
	- Синхронизация системного времени
	- Отправка массива измеренных весов
  * @param sendMessageType Тип сообщения, отправляемого микроконтроллеру ESP
  * @retval Отсутствует
  */
void sendMsgToESP(EspMsg_t sendMessageType){
	switch(sendMessageType){
			case WEIGHT_BUFFER_READY:
				currentCmdESP = 4;
				HAL_UART_Transmit(pUart,sendWeightPrepareCmd,8, 1000);
				//HAL_UART_Transmit_IT(&huart4,sendWeightPrepareCmd,8);		
			break;
			case CHECK_FW:
				checkFWCmd[1] = (versionNum&0xff00)>>8;
				checkFWCmd[2] = versionNum&0xff;
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
/**
	* @brief Задача для отправки сообщений микроконтроллеру ESP	
  * @details Задача производит отправку сообщений для микроконтроллера ESP.
	Сообщения	принимаются через очередь сообщений. Также задача производит приём сообщений от ESP 
  * @param argument Указатель на структуру интерфейса UART, подключенного к ESP
  * @retval Отсутствует
  */
void sendMsgToESPTask(void *argument){	
	static uint8_t messageReceived = 0;
	static uint8_t ingnoreCounter = 0;
	EspMsg_t sendMessageType = EMPTY_MSG;
	
	pUart = (UART_HandleTypeDef*) argument;	
	HAL_GPIO_WritePin(GPIOA, ESP_EN_Pin, GPIO_PIN_SET);
	
	for(;;)
	{			
		if(osMessageQueueGet (espSendQueueHandle, &sendMessageType, 0, MAX_DELAY) == RECEIVE_OK){
			if((currentCmdESP == 4)&&(sendMessageType!=SEND_WEIGHT)){
				ingnoreCounter++;
				if(ingnoreCounter>3) {ingnoreCounter = 0; currentCmdESP = 0;}
				continue;	//ingnore other messages, if send weight process started				
			}
			HAL_UART_Receive_DMA(pUart,receiveBuffer,8);
			sendMsgToESP(sendMessageType);			
			if(osMessageQueueGet (espReceiveQueueHandle, &messageReceived, 0, 1000) == RECEIVE_OK)
				readEspResponse(receiveBuffer);	
			else {
				HAL_UART_DMAStop(pUart);
				if(currentCmdESP == 4){
					currentCmdESP = 0;
					setAdcState(ADC_FREE);
				}
			}
		}		
		osDelay(1);		
	}
}
