/* @file           : rfid.c
 * @date 02.04.2023
 * @author Kamalov Marat
 */
#include "rfid.h"
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "adc.h"
#include "uartCallback.h"
#include "string.h"
// #include "app.h"

extern IWDG_HandleTypeDef hiwdg;
osMessageQueueId_t rfidReceiveQueueHandle;

#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL 0x8408
#define RECEIVE_OK 0
#define TRANSMIT_OK 0
#define GET_READER_INFO_CMD 0x21
#define INVENTORY_G2_CMD 0x01

static UART_HandleTypeDef *pUart;
static uint8_t outputBuffer[50];
static uint8_t inputBuffer[50];
static uint8_t receiveStage = 0;
static int8_t rfidInitState = RFID_ERROR;
static uint8_t currentTag[6] = {0};
static uint8_t deviceAddress = 0;
static uint8_t currentCmdRFID = 0;
static uint8_t tagsBuffer[5][6];
static uint8_t calibrationTag[6] = {1, 2, 3, 4, 5, 6};
static uint8_t offsetTag[6] = {6, 5, 4, 3, 2, 1};
// static const uint8_t calibrationMsg = 1;

uint16_t uiCrc16Calc(uint8_t *buf, uint8_t length)
{
    uint8_t ucI, ucJ;
    uint16_t uiCrcValue = PRESET_VALUE;

    for (ucI = 0; ucI < length; ucI++)
    {
        uiCrcValue = uiCrcValue ^ *(buf + ucI);
        for (ucJ = 0; ucJ < 8; ucJ++)
        {
            if (uiCrcValue & 0x0001)
                uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
            else
                uiCrcValue = (uiCrcValue >> 1);
        }
    }
    return uiCrcValue;
}

void setCRC16(uint8_t *buf)
{
    uint16_t crc = uiCrc16Calc(buf, buf[0] - 1);
    buf[buf[0] - 1] = crc & 0xff;
    buf[buf[0]] = (crc & 0xff00) >> 8;
}

void getReaderInfo(uint8_t *buf)
{
    buf[0] = 4;
    buf[1] = 0xff;
    buf[2] = 0x21;
    setCRC16(buf);
}

void writeCMDToBuf(uint8_t *buf, uint8_t cmdNum)
{
    switch (cmdNum)
    {
    case GET_READER_INFO_CMD:
        buf[0] = 4;
        buf[1] = 0xff;
        buf[2] = cmdNum;
        break;
    case INVENTORY_G2_CMD:
        buf[0] = 13;
        buf[1] = deviceAddress;
        buf[2] = cmdNum;
        buf[3] = 4;  // QValue
        buf[4] = 0;  // Session S0
        buf[5] = 1;  // MaskMem
        buf[6] = 0;  // MaskAdr
        buf[7] = 0;  // MaskAdr
        buf[8] = 0;  // MaskLen
        buf[9] = 0;  // MaskData
        buf[10] = 0; // AdrTID
        buf[11] = 0; // LenTID
        break;
    }
    setCRC16(buf);
    currentCmdRFID = buf[2];
}

void sendCmd(uint8_t *buf, uint8_t cmdNum)
{
    writeCMDToBuf(buf, cmdNum);
    HAL_UART_Receive_DMA(pUart, inputBuffer, 1);
    if (HAL_UART_Transmit(pUart, buf, buf[0] + 1, 1000) == TRANSMIT_OK)
        setReceiveStage(1);
}

int8_t checkBufCRC(uint8_t *buf)
{
    uint16_t crcReceive;

    uint16_t crcCalc = uiCrc16Calc(buf, buf[0] - 1);
    crcReceive = (buf[buf[0]] << 8) | buf[buf[0] - 1];
    if (crcCalc == crcReceive)
        return RFID_OK;
    return RFID_ERROR;
}

uint8_t isTagsEqual(uint8_t *newTag, uint8_t *currentTag)
{
    uint8_t i = 0;

    for (i = 0; i < 6; i++)
    {
        if (newTag[i] != currentTag[i])
            return 0;
    }
    return 1;
}

void readEPCData(uint8_t *buf)
{
    static uint8_t i = 0, j, tempBuf[7], tagsNum;
    static AdcMsg_t adcMsg;

    tagsNum = buf[5];
    while (tagsNum > 0)
    {
        for (j = 0; j < 7; j++)
            tempBuf[j] = buf[10 + j];
        for (j = 0; j < 6; j++)
            tagsBuffer[i][j] = ((tempBuf[j] & 0x0f) << 4) | ((tempBuf[j + 1] & 0xf0) >> 4);
        tagsNum--;
        i++;
    }
    if (buf[5] > 0)
    {
        if (memcmp(&tagsBuffer[i - 1][0], currentTag, 6) != 0)
        {
            if (getAdcState() == ADC_FREE)
            {
                memcpy(currentTag, &tagsBuffer[i - 1][0], 6);
                if (memcmp(currentTag, calibrationTag, 6) == 0)
                {
                    adcMsg = CALIBRATION;
                    osMessageQueuePut(adcQueueHandle, &adcMsg, 0, 0);
                }
                else if (memcmp(currentTag, offsetTag, 6) == 0)
                {
                    adcMsg = SET_OFFSET;
                    osMessageQueuePut(adcQueueHandle, &adcMsg, 0, 0);
                }
                else
                {
                    adcMsg = READ_WEIGHT;
                    //osMessageQueuePut(adcQueueHandle, &adcMsg, 0, 0);
                }
            }
        }
    }
}

void readRfidResponse(uint8_t *buf)
{
    if ((checkBufCRC(buf) == RFID_OK) && (currentCmdRFID == buf[2]))
    {
        switch (buf[2])
        {
        case GET_READER_INFO_CMD:
            deviceAddress = buf[1];
            rfidInitState = RFID_OK;
            break;
        case INVENTORY_G2_CMD:
            if ((buf[3] == 1) && (buf[5] > 0))
                readEPCData(buf);
            break;
        }
    }
}

void rfidInit(void)
{
    HAL_GPIO_WritePin(RFID_EN_GPIO_Port, RFID_EN_Pin, GPIO_PIN_SET);
    osDelay(2000);	
    sendCmd(outputBuffer, GET_READER_INFO_CMD);		
    if (osMessageQueueGet(rfidReceiveQueueHandle, &receiveStage, 0, 2000) == RECEIVE_OK)
    {
        HAL_UART_Receive_DMA(pUart, inputBuffer + 1, inputBuffer[0]);
        if (osMessageQueueGet(rfidReceiveQueueHandle, &receiveStage, 0, 2000) == RECEIVE_OK)
            readRfidResponse(inputBuffer);
        else
            HAL_UART_DMAStop(pUart);
    }
    else
        HAL_UART_DMAStop(pUart); // HAL_UART_Receive_DMA
                                 // writeCMDToBuf(outputBuffer, GET_READER_INFO_CMD);
                                 //->getReaderInfo(outputBuffer);
                                 // HAL_UART_Transmit_IT(&huart6,outputBuffer,outputBuffer[0]+1);
                                 // receiveStart = 1;
}

void rfidReInit(void)
{
    HAL_GPIO_WritePin(RFID_EN_GPIO_Port, RFID_EN_Pin, GPIO_PIN_RESET);
    osDelay(1000);
    rfidInit();
}

void readRfidTask(void *argument)
{
    /* USER CODE BEGIN readRfid */
    uint8_t initCounter = 0;
    pUart = (UART_HandleTypeDef *)argument;

    rfidInit();
    while (rfidInitState != RFID_OK)
    {
        osDelay(1000); // mytest add time management
        initCounter++;
        if (initCounter >= 255)
        {
            initCounter = 0;
            rfidReInit();
        }
    }
    /* Infinite loop */
    for (;;)
    {
        osDelay(1000);
        sendCmd(outputBuffer, INVENTORY_G2_CMD);
        HAL_IWDG_Refresh(&hiwdg);
        if (osMessageQueueGet(rfidReceiveQueueHandle, &receiveStage, 0, 2000) == RECEIVE_OK)
        {
            if (receiveStage == 1)
            {
                HAL_UART_Receive_DMA(pUart, inputBuffer + 1, inputBuffer[0]);
                if (osMessageQueueGet(rfidReceiveQueueHandle, &receiveStage, 0, 2000) == RECEIVE_OK)
                    readRfidResponse(inputBuffer);
                else
                    HAL_UART_DMAStop(pUart);
            }
            else
                HAL_UART_DMAStop(pUart);
        }
        else
            HAL_UART_DMAStop(pUart);
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
    }
    /* USER CODE END readRfid */
}

void getCurrentTag(uint8_t *tagBuf)
{
    memcpy(tagBuf, currentTag, 6);
}
