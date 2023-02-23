#ifndef POST_REQ
#define POST_REQ

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "cJSON.h"


void post_rest_function();


typedef struct
{
    char Scales_Serial_Number[25];
    unsigned long Weighting_Start;
    unsigned long Weighting_End;
    char RFID_Number[15];
    float array_of_weights[60];
    uint8_t size_of_float_array;
}POST_DATA;

extern POST_DATA prepare_to_json;
extern char *json_string_send_to_sever;
extern int STATUS_CODE;

#endif