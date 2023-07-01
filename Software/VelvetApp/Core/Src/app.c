 /**
  * @file app.c
	* @brief �������� ��� ������� ������� ����� �� ��������
  * @date 23.02.2023  
  * @author Kamalov Marat  
  */

#include "app.h"
#include "cmsis_os2.h"
#include "esp.h"

volatile uint16_t secondsCounter;
volatile uint32_t timestamp = 0;
static EspMsg_t espMsg;

/**
	* @brief ������� ������� ����� �� ��������
	* @details 1 ��� � 15 ����� ���������� ������������� ���������� �������
						 1 ��� � ������ ���������� �������� ������� ���������� ��
  * @param �����������
  * @retval �����������
  */
void checkTasksTimeout(){
	secondsCounter++;
	timestamp++;
	
	if(secondsCounter>TIME_SYNCHRO_TIMEOUT){
		secondsCounter=0;
		espMsg = TIME_SYNCHRONIZATION;
		osMessageQueuePut(espSendQueueHandle, &espMsg, 0, 0);		
	}	
	if(secondsCounter%CHECK_FW_TIMEOUT == 0){
		espMsg = CHECK_FW;
		osMessageQueuePut(espSendQueueHandle, &espMsg, 0, 0);
	}
}
