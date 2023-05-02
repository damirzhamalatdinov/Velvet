/* @file           : rfid.h
  * @date 02.04.2023  
  * @author Kamalov Marat  
  */
	
#ifndef __RFID_H
#define __RFID_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

extern uint8_t outputBuffer[50];
extern uint8_t inputBuffer[50];
extern uint8_t receiveStage;
extern uint8_t rfidInitState;
extern uint8_t currentTag[6];

#define RFID_ERROR					   -1
#define RFID_OK						    0

void readRfidResponse(uint8_t* buf);

#ifdef __cplusplus
}
#endif

#endif /* __RFID_H */