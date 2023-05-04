/* @file           : esp.h
  * @date 12.03.2023  
  * @author Kamalov Marat  
  */
	
#ifndef __ESP_H
#define __ESP_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

void sendMsgToESP(void *argument);
void readEspResponse(uint8_t* buf);
#ifdef __cplusplus
}
#endif

#endif /* __ESP_H */