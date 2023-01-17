#include "adc.h"

void delay_us(uint8_t value){
    uint8_t i;
    for(i=value; i > 0; i--);
}

// FDSFSD
float convertToKg(unsigned long count){
	float weight = (float)count/258;
	return  weight;
}

float HX711_Read (void) // усиление 128
{
	unsigned long count = 0; 
	unsigned char i; 	
  	HAL_GPIO_WritePin (GPIOD, GPIO_PIN_6, GPIO_PIN_RESET); // Когда модуль не готов, микроконтроллер выдает низкий уровень на линии CLK
	int ii = 0;
    while(1)
	{
	    if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7) == 0)
		{
			delay_us(1);
			if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7) == 0)
			{
				break;
			}			
		}
			
		ii++;
		if(ii >= 1000000)
		{
			break;
		}
	}
	delay_us(1);
  	for(i = 0; i < 24; i++)
	{ 			
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6, GPIO_PIN_SET);
		delay_us(1);
	  	count = count << 1; 	
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6, GPIO_PIN_RESET);		
	  	 if (HAL_GPIO_ReadPin (GPIOD, GPIO_PIN_7) == 1) // Чтение данных
		{
			count++; 
		}		
			
	} 
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6, GPIO_PIN_SET);
	delay_us(2);
         count = count ^ 0x800000; // Когда наступает 25-й импульс падения, преобразуйте данные
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6, GPIO_PIN_RESET);

	float weight = convertToKg(count);
	return weight;
}

uint16_t GetAnimalWight(){
	int buf_size = 10;
	float weight_buf[buf_size];
	float weight_sum = 0;
	int idx = 0;

	while (idx < buf_size){
		weight_buf[idx] = HX711_Read();
		idx++;
		HAL_Delay(200);
	}
	
	uint8_t count = 0;
	for(int i = 0; i < buf_size; i++){
		if(weight_buf[i] > 100 && weight_buf[i] < 800){
			weight_sum += weight_buf[i];
			count++;
		}
	}

	if (count == 0){
		return 0;
	}

	return weight_sum/count;
}


void ScalesCalibrate(){

}