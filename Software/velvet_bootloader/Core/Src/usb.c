#include "usb.h"

void USBPrint(uint8_t *buf)
{
  HAL_Delay(50);
  CDC_Transmit_FS(buf, strlen(buf));
}


void USBprintArray(uint8_t *buf, uint8_t size){
  uint8_t out[10];
  sprintf(out, "Size: %d [", size);
  USBPrint(out);
  // USBPrint("[");
  for(int i = 0; i < size; ++i){
    sprintf(out, "%X, ", buf[i]);
    USBPrint(out);
  }
  USBPrint("]\n");
}