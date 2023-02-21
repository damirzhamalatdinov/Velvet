#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

#define UART UART_NUM_2

void initialize_uart();
void tx_task(void *arg);
void rx_task(void *arg);
extern uint8_t *data;