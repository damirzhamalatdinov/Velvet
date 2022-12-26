#include "rfid.h"

unsigned int uiCrc16Cal(unsigned char const  * pucY, unsigned char ucX)
{
	unsigned char ucI,ucJ;
	unsigned short int  uiCrcValue = PRESET_VALUE;

   	for(ucI = 0; ucI < ucX; ucI++)
	   {
		   uiCrcValue = uiCrcValue ^ *(pucY + ucI);
	  	   for(ucJ = 0; ucJ < 8; ucJ++)
	   	  {
		 	if(uiCrcValue & 0x0001)
		   	{
		    	uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
		   	}
		 	else
		   	{
		    	uiCrcValue = (uiCrcValue >> 1);
		   	}
		}
 	}
    return uiCrcValue;
}


bool isCRCValid( uint8_t const *data){
    int data_length = data[0] + 1;
    int crc = uiCrc16Cal(data, data_length - 2);
    if ((data[data_length - 2] == (crc & 0xff)) && (data[data_length - 1] == (crc >> 8)))
        return 1;
    return 0;
}


void addCRC16(uint8_t *buf){
    int data_length = buf[0] + 1;
    int crc = uiCrc16Cal(buf, data_length - 2);
    buf[data_length - 2] = (crc & 0xff);
    buf[data_length - 1] = (crc >> 8);
}


bool RFIDReadTag(UART_HandleTypeDef *huart, struct TagData *buf){
    uint8_t cmd[5] = {0x04, RFID_ADDRESS, G2_SINGLE_TAG_INVENTORY};
    addCRC16(cmd);
    HAL_UART_Transmit(huart, cmd, 5, 2000);
    HAL_UART_Receive(huart, buf->id , 50, 2000);
    
    if (buf->id[2] != G2_SINGLE_TAG_INVENTORY ){
        return false;
    } else {
        if (isCRCValid(buf) && buf->id[5] == 1){
            buf->length = buf->id[6];
            for(int i = 0; i < buf->length; ++i){
                buf->id[i] = buf->id[7 + i];
            }
            return true;
        } else {
            buf->id[0] = 0xf2;
            return false;
        }
    }
    buf->id[0] = 0xf3;
    return false;
} 


void resetTagData(struct TagData *tag){
    for(int i = 0; i < 50; i++){
        tag->id[i] = 0;
    }
    tag->length = 0;
}