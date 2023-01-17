/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RFID_H__
#define __RFID_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdbool.h>

#define RFID_ADDRESS 0xFF
#define RFID_OK_STATUS 0x00
#define RFID_READ_OK_STATUS 0x01


#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL 0x8408
#define G2_TAG_INVENTORY 0x01
#define G2_READ_DATA 0x02
#define G2_WRITE_DATA 0x03
#define G2_WRITE_EPC 0x04
#define G2_KILL_TAG 0x05
#define G2_SET_PROTECTION 0x06
#define G2_ERASE_BLOCK 0x07
#define G2_READ_PROTECTION_EPC 0x08
#define G2_READ_PROTECTION_NO_EPC 0x09
#define G2_UNLOCK_READ_PROTECTION 0x0a
#define G2_READ_PROTECTION_STATUS_CHECK 0x0b
#define G2_EAS_CONFIGURATION 0x0c
#define G2_EAS_ALERT_DETECTION 0x0d
#define G2_SINGLE_TAG_INVENTORY 0x0f
#define G2_WRITE_BLOCKS 0x10
#define G2_GET_MONZA_4QT_WORKING_PARAMETERS 0x11
#define G2_SET_MONZA_4QT_WORKING_PARAMETERS 0x12
#define G2_READ_EXTENDED_DATA 0x15
#define G2_WRITE_EXTENDED_DATA 0x16
#define G2_TAG_INVENTORY_WITH_MEMORY_BUFFER 0x18
#define G2_MIX_INVENTORY 0x19
#define G2_INVENTORY_EPC 0x1a
#define G2_QT_INVENTORY 0x1b

#define CF_GET_READER_INFO 0x21
#define CF_SET_WORKING_FREQUENCY 0x22
#define CF_SET_READER_ADDRESS 0x24
#define CF_SET_READER_INVENTORY_TIME 0x25
#define CF_SET_SERIAL_BAUD_RATE 0x28
#define CF_SET_RF_POWER 0x2f
#define CF_SET_WORK_MODE_288M 0x76
#define CF_SET_WORK_MODE_18 0x35
#define CF_SET_BUZZER_ENABLED 0x40
#define CF_SET_ACCOUSTO_OPTIC_TIMES 0x33

struct TagData {
    uint8_t id[50];
    uint8_t length;
};

bool isCRCValid( uint8_t const *data);
bool RFIDReadTag(UART_HandleTypeDef *huart, struct TagData *buf);
unsigned int uiCrc16Cal(unsigned char const  * pucY, unsigned char ucX);
void addCRC16(uint8_t *buf);
void resetTagData(struct TagData *tag);

#ifdef __cplusplus
}
#endif

#endif /* __USB_H__ */