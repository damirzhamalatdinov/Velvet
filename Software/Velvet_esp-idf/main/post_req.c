#include "post_req.h"
static const char *TAG = "POST";
// POST_DATA prepare_to_json;
// char *json_string_send_to_sever;
// int STATUS_CODE;

esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}

void create_data_to_post(POST_DATA *some_data)
{
    float const array[] = {100.01, 123.1, 345.2, 678.3};
    strcpy(some_data->Scales_Serial_Number, "Facebook");
    some_data->Weighting_Start = 1000;
    some_data->Weighting_End = 2000;
    strcpy(some_data->RFID_Number, "101010101010");
    for (int i = 0; i < 4; i++){
        some_data->array_of_weights[i]=array[i];
    }
    some_data->size_of_float_array = 4;
}

static void convert_struct_to_json(POST_DATA *some_data)
{
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "ScalesSerialNumber", (*some_data).Scales_Serial_Number);
    cJSON_AddNumberToObject(root, "WeighingStart", (*some_data).Weighting_Start);
    cJSON_AddNumberToObject(root, "WeighingEnd", (*some_data).Weighting_End);
    cJSON_AddStringToObject(root, "RFIDNumber", (*some_data).RFID_Number);

    float f_numbers[(*some_data).size_of_float_array];
    for (int i = 0; i < (*some_data).size_of_float_array; i++) {
        f_numbers[i] = (*some_data).array_of_weights[i];
    }
    cJSON *floatArray;
    floatArray = cJSON_CreateFloatArray(f_numbers, ((*some_data).size_of_float_array));
    cJSON_AddItemToObject(root, "Data", floatArray);
    
    json_string_send_to_sever = cJSON_Print(root);
    //printf("%s", json_string_send_to_sever);
    cJSON_Delete(root);
}

void post_rest_function()
{   
    create_data_to_post(&prepare_to_json);
    convert_struct_to_json(&prepare_to_json);
    printf("%s", json_string_send_to_sever);
    printf("%d", strlen(json_string_send_to_sever));
    esp_err_t err;
    esp_http_client_config_t config_post = {
        .url = "https://smart-farm.kz:8502/v2/OneTimeWeighings",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    esp_http_client_set_post_field(client, json_string_send_to_sever, strlen(json_string_send_to_sever));
    esp_http_client_set_header(client, "Content-Type", "application/json; charset=utf-8");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    STATUS_CODE = esp_http_client_get_status_code(client);
}

static char *JSON_Types(int type) {
    if (type == cJSON_Invalid) return ("cJSON_Invalid");
    if (type == cJSON_False) return ("cJSON_False");
    if (type == cJSON_True) return ("cJSON_True");
    if (type == cJSON_NULL) return ("cJSON_NULL");
    if (type == cJSON_Number) return ("cJSON_Number");
    if (type == cJSON_String) return ("cJSON_String");
    if (type == cJSON_Array) return ("cJSON_Array");
    if (type == cJSON_Object) return ("cJSON_Object");
    if (type == cJSON_Raw) return ("cJSON_Raw");
    return NULL;
}

static int JSON_Array(const cJSON * const item) {
    if (cJSON_IsArray(item)) {
        cJSON *current_element = NULL;
        int itemNumber = 0;
        cJSON_ArrayForEach(current_element, item) {
            //ESP_LOGI(TAG, "current_element->type=%s", JSON_Types(current_element->type));
            if (cJSON_IsNumber(current_element)) {
                int valueint = current_element->valueint;
                double valuedouble = current_element->valuedouble;
                ESP_LOGI(TAG, "valueint[%d]=%d valuedouble[%d]=%f",itemNumber, valueint, itemNumber, valuedouble);
            }
            if (cJSON_IsString(current_element)) {
                const char* string = current_element->valuestring;
                ESP_LOGI(TAG, "string[%d]=%s",itemNumber, string);
            }
            itemNumber++;
        }
        return cJSON_GetArraySize(item);
    } else {
        return 0;
    }
}


