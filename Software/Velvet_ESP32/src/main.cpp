
/**
 * @mainpage Velvet_ESP32
 * @brief Проект Velvet_ESP32 предназначен для реализации протокола общения между микроконтроллерами ESP32 и STM32
 * с использованием интерфейса UART. Проект содержит файлы ESP32_STM_UART.h и Settings_ESP.h.
 * @section structure Структура проекта
 * * main.cpp
 * * ESP32_STM_UART.h
 * * ESP32_STM_UART.cpp
 * * Settings_ESP.h
 * @section protocol Протокол общения
 * Протокол общения между устройствами STM и ESP32 описан в документации проекта.

 * @details Протокол общения между устройствами представляет собой набор команд, передаваемых по UART. Команды от STM32
 * содержат определенные значения, указывающие на тип команды и ее параметры. Команды от ESP32 являются ответами на
 * соответствующие команды от STM32 и содержат информацию о выполнении команды и ее результатах.
 * В библиотеке ESP32_STM_UART реализованы методы для обработки входящих команд, отправки их на STM32, а также для
 * обработки ответов от STM32. Кроме того, библиотека содержит дополнительные методы для работы с UART, такие как
 * настройка скорости передачи данных и установка пинов для подключения интерфейса UART.
 * Для работы с библиотекой необходимо создать объект класса ESP32_STM_UART и вызвать методы для обработки команд и
 * отправки ответов. Подробная документация по каждому методу доступна в соответствующих файлах с исходным кодом.
*/
/**

 * @file main.cpp
 * @brief Главный файл проекта ESP32_STM_UART. Точка входа в программу
 * @details В функции setup() происходит инициализация устройств и настройка параметров UART. В функции loop()
 * происходит обработка входящих команд и отправка ответов на STM32.
 * 
*/
#include <Arduino.h>
#include <WiFi.h>
#include "ESP32_STM_UART.h"
#include "Settings_ESP.h"
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";
bool debug_flag = true;

//HardwareSerial Serial2(2);
uint32_t uartBaudRate = 115200; 

/**
 * @brief Создает экземпляр ESP32_STM_UART для обработки сообщений.
 * @param Serial2 Аппаратный последовательный порт, используемый для общения.
 * @param uartBaudRate Скорость передачи данных в бод (baud rate) для последовательного порта.
 */

ESP32_STM_UART stmUart(Serial2, uartBaudRate);

/**
 * @brief Настраивает начальные параметры, такие как последовательный порт и подключение Wi-Fi.
 */
void setup() {
    Serial.begin(uartBaudRate);
    
    //WiFi.begin(ssid, password);

    uint8_t retries = 5;
    /**
     * @brief Подключение к wi-fi retries раз
    */
    if (!stmUart.check_wifi()) {
        /**
         * @brief Не удалось подключиться к Wi-Fi, здесь можно добавить обработку ошибок
        */ 
        // Serial.println("Не удалось подключиться к Wi-Fi");
    }
}

/**
 * @brief Основной цикл программы, обрабатывает входящие сообщения через stmUart.
 */
void loop() {
    stmUart.processIncomingMessage();
}