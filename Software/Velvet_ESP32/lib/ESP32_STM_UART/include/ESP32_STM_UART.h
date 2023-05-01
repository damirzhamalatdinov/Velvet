/**
 * @file ESP32_STM_UART.h
 * @brief Заголовочный файл класса ESP32_STM_UART для обработки коммуникации с STM32.
*/

#ifndef ESP32_STM_UART_H
#define ESP32_STM_UART_H

#include <Arduino.h>
#include <HardwareSerial.h>

/**
 * @class ESP32_STM_UART
 * @brief Класс для обработки коммуникации с STM32 по протоколу, основанному на UART.
 * Данный класс реализует протокол обмена данными между микроконтроллерами ESP32 и STM32 с помощью интерфейса UART.
 * Он предоставляет ряд методов для отправки и получения команд и данных, в том числе:
 * * проверка наличия подключения к Wi-Fi;
 * * обработка входящих сообщений;
 * * отправка данных на сервер;
 * * проверка версии прошивки;
 * * отправка ответов на запросы передачи весовых данных;
 * * отправка метки времени в формате unix timestamp;
 * * передача данных прошивки на STM.
*/

class ESP32_STM_UART {
  public:
/**
 * @brief Конструктор класса
 * @param serial Интерфейс UART, который будет использоваться для коммуникации.
 * @param baudRate Скорость передачи данных в бодах.
*/
  ESP32_STM_UART(HardwareSerial &serial, uint32_t baudRate);
/**
 * @brief Проверяет наличие подключения к Wi-Fi. 
 * @return Логическое значение, указывающее наличие подключения к Wi-Fi.
*/
  bool check_wifi();
/**
 * @brief Обрабатывает входящие сообщения.
 * Метод используется для обработки входящих сообщений от STM32. Он определяет тип сообщения и вызывает соответствующую
 * функцию для обработки сообщения.
*/
  void processIncomingMessage();

private:
  HardwareSerial &_serial; ///< Интерфейс UART для общения с STM32.
  uint32_t _baudRate; ///< Скорость передачи данных в бодах.
  uint8_t* _firmwareData; ///< Указатель на буфер данных прошивки.
  size_t _firmwareSize; ///< Размер данных прошивки.

/**
 * @brief Отправляет данные на сервер.
 * @param cowId Идентификатор коровы.
 * @param ts Временная метка в формате unix timestamp.
 * @param weight Массив весовых данных.
 * @param weightCount Количество элементов в массиве весовых данных.
 * @return Логическое значение, указывающее на успешность отправки данных на сервер.
*/
  bool sendDataToServer(uint8_t *cowId, uint32_t ts, float *weight, uint8_t weightCount);

/**
 * @brief Проверяет соответствие версии прошивки.
 * Эта функция проверяет наличие новой версии прошивки на сервере.
 * @param version0 Младший байт версии прошивки, переданный из STM32.
 * @param version1 Старший байт версии прошивки, переданный из STM32.
*/
  void checkFirmware(uint8_t version0, uint8_t version1);

/**
 * @brief Отправляет ответ на команду отправки весовых данных.
 * Эта функция отправляет ответ на команду отправки весовых данных. Принимает результат выполнения команды.
 * @param result Результат выполнения команды.
*/
  void sendWeightRsp(uint8_t result);
/**
 * @brief Отправляет ответ с меткой времени.
 * Эта функция отправляет ответ STM32 с меткой времени в формате unix timestamp и указанным результатом.
 * @param timestamp Метка времени в формате unix timestamp.
 * @param result Результат выполнения команды.
*/
  void sendTimestampRsp(uint32_t timestamp, uint8_t result);
/**

 * @brief Отправляет команду на подготовку передачи весовых данных.
 * Эта функция отправляет команду STM32 на подготовку передачи весовых данных.
*/
  void sendWeightCmdTransmitPrepare();
  
/**
 * @brief Отправляет прошивку на STM32.
 * Эта функция отправляет прошивку на STM32.
 * @param firmwareData Данные прошивки.
 * @param firmwareSize Размер прошивки.
*/
  void sendFirmwareToSTM(const uint8_t *firmwareData, size_t firmwareSize);
  
/**
 * @brief Отправляет ответ на команду CHECK_FW_CMD.
 * Эта функция отправляет ответ STM32 на команду CHECK_FW_CMD с указанным результатом.
 * @param result Результат выполнения команды.
*/
  void sendCheckFwRsp(uint8_t result);
  
/**
 * @brief Функция вычисления контрольной суммы CRC32
 * Функция вычисляет контрольную сумму CRC32 для переданного скачанного файла с сервера.
 * @param data Указатель на буфер данных для вычисления CRC32
 * @param dataSize Размер буфера данных
 * @return Строка, содержащая вычисленную CRC32 в шестнадцатеричном формате.
*/
  String calculateCRC32(uint8_t *data, uint32_t dataSize);

/**
 * @brief Функция получения текущего времени в формате Unix timestamp
 * Функция получает текущее время в формате Unix timestamp (количество секунд, прошедших с 1 января 1970 года).
 * @return Текущее время в формате Unix timestamp.
*/
  uint32_t getUnixTimestamp();
  
/**
 * @brief Перечисление типов сообщений, используемых в протоколе общения между ESP32 и STM32
 * Это перечисление определяет типы сообщений, которые используются для обмена данными между ESP32 и STM32 в протоколе общения.
 * Оно определяет значения, используемые в поле типа сообщения пакета.
*/
  enum MessageType {
    CHECK_FW_CMD = 0x01,
    BOOTLOADER_READY_CMD = 0x02,
    READY_TO_RECEIVE_DATA_CMD = 0x03,
    SEND_WEIGHT_CMD_TRANSMIT_PREPARE = 0x04,
    SEND_WEIGHT_CMD_TRANSMIT_WEIGHT_ARRAY = 0x05,
    GET_TIMESTAMP_CMD = 0x06
  };
};

#endif