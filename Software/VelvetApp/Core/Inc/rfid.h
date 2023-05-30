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

#define RFID_ERROR					   -1
#define RFID_OK						    0

void readRfidTask(void *argument);
void getCurrentTag(uint8_t* tagBuf);

#ifdef __cplusplus
}
#endif

#endif /* __RFID_H */