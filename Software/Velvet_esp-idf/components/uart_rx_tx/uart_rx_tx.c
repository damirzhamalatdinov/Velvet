#include <stdio.h>
#include "uart_rx_tx.h"

static const int RX_BUF_SIZE = 1024;

uint8_t *uart_rx_data;
int num = 0;

void initialize_uart(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void tx_task(void *arg)
{
    static const char *TAG = "TX_TASK";
    esp_log_level_set(TAG, ESP_LOG_INFO);
    char *Txdata = (char*)malloc(30);
 
    while (1) {
        sprintf(Txdata, "Insert value here %d\r\n", num++); // I need to think here
        uart_write_bytes(UART, (const char*) Txdata, strlen(Txdata));        
        ESP_LOGI(TAG, "TXDATA IS: %s", Txdata);
        //vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    free(Txdata);
}

void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uart_rx_data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 500 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            uart_rx_data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, (char*)uart_rx_data);
            //ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        }
    }
    free(data);
}


void test(void) // insert to app_main() like here
{
    init();
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}
