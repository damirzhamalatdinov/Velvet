/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_H__
#define __USB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_cdc_if.h"

void USBPrint(uint8_t *buf);
void USBprintArray(uint8_t *buf, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* __USB_H__ */