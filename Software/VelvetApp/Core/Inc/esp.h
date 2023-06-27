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
#include "cmsis_os2.h"

typedef enum{
	EMPTY_MSG,
	CHECK_FW,
	TIME_SYNCHRONIZATION,
	WEIGHT_BUFFER_READY,
	SEND_WEIGHT
} EspMsg_t;
/* Definitions for espSendQueue */
extern osMessageQueueId_t espSendQueueHandle;
/* Definitions for espReceiveQueue */
extern osMessageQueueId_t espReceiveQueueHandle;

void sendMsgToESPTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __ESP_H */