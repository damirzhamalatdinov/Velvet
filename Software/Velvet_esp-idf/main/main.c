#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "time_synch.h"
#include "esp_netif_sntp.h"
#include "connect_wifi.h"
#include "post_req.h"

static const char *TAG = "MAIN";


void send_weight_data_to_server(void *pvParameters){
	// JSON_STRING = 
	//     "{\"ScalesSerialNumber\": \"velvet_test_max\", \"WeighingStart\": 1000, \"WeighingEnd\": end_time,\n  "
    //     "\"RFIDNumber\": \"001001001001\", \"Data\": [100, 200, 300, 100]}";
	//esp_err_t err;
	
	//vTaskDelay(5000/portTICK_PERIOD_MS);
	//snprintf(JSON_STRING, sizeof(JSON_STRING));
	post_rest_function();
	if (STATUS_CODE == 200)
		ESP_LOGI(TAG, "200");
	else ESP_LOGI(TAG, "NO");
	//ent_time++;
	vTaskDelete(NULL);
}

void get_timestamp(void *params){
	while(1){
		ESP_LOGI(TAG, "Time Stamp is: %llu", TimeStamp);
		vTaskDelay(3000/portTICK_PERIOD_MS);
		}
}

void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	connect_wifi();
	initialize_sntp();
	if (wifi_connect_status)
	{		
		//xTaskCreate(&print_datetime_n, "Current_Date_Time", 2048, NULL, 1, NULL);
		//xTaskCreate(&datetime_now, "print_datetime", 8192, NULL, 6, NULL);
		//xTaskCreate(&get_timestamp, "get_timestamp", 2048, NULL, 5, NULL);
		
		xTaskCreate(&send_weight_data_to_server, "send_weight_data_to_server", 8192, NULL, 1, NULL);
	}	
}