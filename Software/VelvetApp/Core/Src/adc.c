/* @file           : adc.c
 * @date 19.04.2023
 * @author Kamalov Marat
 */
#include "adc.h"
#include "hx711.h"
#include "cmsis_os2.h"
// #include "stm32f4xx_hal.h"
// #include "main.h"
#include "app.h"
#include "esp.h"

#define RECEIVE_OK 0
#define SAMPLE_NUMBER 10

osMessageQueueId_t adcQueueHandle;
static EspMsg_t espmsg;
static hx711_t loadcell;
static float weightBuffer[60];
static uint8_t weightIndex = 0;
static uint8_t adcState = ADC_FREE;

void initADC(void)
{
	hx711_init(&loadcell, HX_SCK_GPIO_Port, HX_SCK_Pin, HX_DOUT_GPIO_Port, HX_DOUT_Pin);
	// hx711_coef_set(&loadcell, 354.5); // read afer calibration
	hx711_coef_set(&loadcell, 1.0); // no calibration, clean adc val
	osDelay(100);
	hx711_tare(&loadcell, SAMPLE_NUMBER);
	// osDelay(5000);
	// hx711_coef_set(&loadcell, hx711_weight(&loadcell, 10)/55);//55 òàðèðîâàííûé âåñ 55 ã
}

void readWeightTask(void *argument)
{
	static AdcMsg_t adcMsg;

	initADC();
	for (;;)
	{
		if (osMessageQueueGet(adcQueueHandle, &adcMsg, 0, MAX_DELAY) == RECEIVE_OK)
		{
			adcState = ADC_BUSY;
			for (weightIndex = 0; weightIndex < 60; weightIndex++)
			{
				osDelay(20); // mytest add time management
				weightBuffer[weightIndex] = hx711_weight(&loadcell, SAMPLE_NUMBER);
			}
			espmsg = WeightBufferReady;
			osMessageQueuePut(espSendQueueHandle, &espmsg, 0, 0);
		}
	}
}

uint8_t getAdcState(void)
{
	return adcState;
}

void setAdcState(uint8_t state)
{
	adcState = state;
}

float getWeightValByIndex(uint8_t index)
{
	return weightBuffer[index];
}