/**
 * @file ESP32_STM_UART.cpp
 * @brief Реализация класса ESP32_STM_UART для обработки коммуникации с STM32.
 */

#include <Arduino.h>
#include "ESP32_STM_UART.h"
#include <time.h>
#include <WiFi.h>
#include "Settings_ESP.h"
#include <HTTPClient.h>
//#include <WiFiClientSecure.h>
#include <zlib.h>
#include <Update.h>

/**
 * @brief Конструктор класса ESP32_STM_UART.
 *
 * Инициализирует объект ESP32_STM_UART и настраивает серийное соединение.
 *
 * @param serial Ссылка на объект HardwareSerial для использования.
 * @param baudRate Скорость передачи данных для серийного соединения.
 * @brief Пример создания объекта 
 * @code 
 *      ESP32_STM_UART stmUart(Serial2, uartBaudRate);
 * @endcode
 */
ESP32_STM_UART::ESP32_STM_UART(HardwareSerial &serial, uint32_t baudRate)
    : _serial(serial), _baudRate(baudRate) {
    _serial.begin(_baudRate);
}

/**
 * @brief Проверяет состояние подключения к Wi-Fi.
 * 
 * Функция выполняет проверку состояния подключения к Wi-Fi сети. Если подключение не установлено, 
 * она пытается подключиться с заданными параметрами SSID и паролем. Функция делает несколько попыток подключения 
 * с задержкой между попытками.
 * 
 * @return Возвращает true, если подключение к Wi-Fi успешно установлено, иначе false.
 */

bool ESP32_STM_UART::check_wifi(){
    uint8_t retries = 5;
    while (WiFi.status() != WL_CONNECTED && retries > 0) {
        delay(500);
        WiFi.begin(ssid, password);
        retries--;
    }
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    return true;
}

/**

 * @details Данная функция читает байты, переданные от STM, и выполняет соответствующие действия в зависимости от кода команды. Команды, которые могут быть обработаны, включают CHECK_FW_CMD, BOOTLOADER_READY_CMD, SEND_WEIGHT_CMD_TRANSMIT_PREPARE и GET_TIMESTAMP_CMD.
 * @return void
 * @note Для корректной обработки команд необходимо, чтобы команды соответствовали протоколу общения между STM и ESP32.
 * @brief Протокол общения между устройствами STM и ESP32
 * @details Команды, передаваемые от STM к ESP32:
 * * {0x01, Version0, Version1, 0x00, 0x00, 0x00, 0x00, 0x00} - CHECK_FW_CMD команда на проверку версии прошивки;
 * * {0x02, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - BOOTLOADER_READY_CMD команда на готовность STM к загрузке новой прошивки;
 * * {0x03, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - READY_TO_RECEIVE_DATA_CMD команда на готовность к получению данных;
 * * {0x04, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - SEND_WEIGHT_CMD_TRANSMIT_PREPARE команда на подготовку к передаче весовых данных;
 * * {0x05, 8B cow ID, 4B ts – unix epoch, 4B x 60 - weight float} - SEND_WEIGHT_CMD_TRANSMIT_WEIGHT_ARRAY команда на передачу весовых данных;
 * * {0x06, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - GET_TIMESTAMP_CMD команда на получение метки времени в формате unix timestamp.
 
 * Команды, передаваемые от ESP32 к STM:
 * * {0x01, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - ответ на команду CHECK_FW_CMD, перезагрузить STM;
 * * {0x01, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - ответ на команду CHECK_FW_CMD, ошибка соединения;
 * *{0x01, 0x03, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - ответ на команду CHECK_FW_CMD, обновлений нет;
 * * {0x02, 2B Size of FW, 0x00, 0x00, 0x00, 0x00, 0x00} - ответ на команду BOOTLOADER_READY_CMD, размер прошивки;
 * * {0x03, 256B FW Data} - ответ на команду SEND_WEIGHT_CMD_TRANSMIT_PREPARE, передача данных прошивки;
 * * {0x04, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - ответ на команду SEND_WEIGHT_CMD_TRANSMIT_PREPARE, сообщает об успешной подготовке к передаче весовых данных.
 * * {0x05, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - ответ на команду SEND_WEIGHT_CMD_TRANSMIT_WEIGHT_ARRAY, сообщает об успешной передаче весовых данных.
 * * {0x05, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - ответ на команду SEND_WEIGHT_CMD_TRANSMIT_WEIGHT_ARRAY, сообщает об ошибке передачи весовых данных по WiFi.
 * * {0x06, 4B ts – unix epoch, 0x00, 0x00, 0x00} - ответ на команду GET_TIMESTAMP_CMD, содержит запрошенную метку времени в формате unix timestamp.
 * * {0x06, 0x02, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - ответ на команду GET_TIMESTAMP_CMD, сообщает об ошибке при запросе метки времени.
*/

void ESP32_STM_UART::processIncomingMessage() {
    if (!_serial.available()) {
        return;
    }
    
    uint8_t cmd = _serial.read();
    
    switch (cmd) {
        case CHECK_FW_CMD: {
            // Обработка CHECK_FW_CMD
            uint8_t version0 = _serial.read();
            uint8_t version1 = _serial.read();
            checkFirmware(version0,version1);
            break;
        }
        case BOOTLOADER_READY_CMD:{
            sendFirmwareToSTM(_firmwareData, _firmwareSize);
            break;
        }
        case SEND_WEIGHT_CMD_TRANSMIT_PREPARE: {
            // Обработка SEND_WEIGHT_CMD_TRANSMIT_PREPARE
            sendWeightCmdTransmitPrepare();
            break;
        }
        case GET_TIMESTAMP_CMD: {
            // Обработка GET_TIMESTAMP_CMD
            uint32_t timestamp = getUnixTimestamp();           
            if (timestamp != 0) {
                sendTimestampRsp(timestamp, 0x00);
            } else {
                sendTimestampRsp(0, 0x02);
            }
            break;
        }
       
    }
}

/**
 * @brief Проверяет и получает обновление прошивки с сервера при наличии новой версии.
 * 
 * Функция проверяет доступность Wi-Fi, а затем отправляет запрос на сервер для получения обновления прошивки.
 * Если сервер возвращает успешный ответ (HTTP-код 200), функция проверяет контрольную сумму (CRC32) полученного файла
 * прошивки и сравнивает ее с контрольной суммой, полученной из заголовка ответа сервера. Если контрольные суммы
 * совпадают, функция сохраняет данные прошивки и отправляет ответ на запрос проверки версии прошивки с результатом
 * "Перезагрузить STM".
 *
 * @param version0 Младший байт версии прошивки, переданный из STM32.
 * @param version1 Старший байт версии прошивки, переданный из STM32.
 */

void ESP32_STM_UART::checkFirmware(uint8_t version0, uint8_t version1) {
    bool wi_fi = check_wifi();
    if (!wi_fi){
        sendCheckFwRsp(0x02);
        return;
    }
    char version_str[8];
    sprintf(version_str, "%d.%d", version0, version1);
    String version = String(version_str);

    //String version = String(version0) + "." + String(version1);
    String serialNumber = "MAX_TEST_ESP32"; // Заменить на реальный серийный номер
    // Получение обновления прошивки с сервера
    WiFiClientSecure wifiClient;
    HTTPClient http;
    String url = "https://smart-farm.kz:8502/api/v2/ActualScalesFirmware";
    String postData = "serialNumber=" + serialNumber + "&firmwareVersion=" + version;
    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/json; charset=utf-8");
    int httpResponseCode = http.POST(postData);
    if (httpResponseCode != 200) {
        sendCheckFwRsp(0x03);
        return;
    }
    // Получение заголовков и содержимого файла
    int contentLength = http.getSize();
    String fileCRC = http.header("CRC");
    String fileVersion = http.header("Version");
    // Создание динамического массива для хранения данных прошивки
    uint8_t *firmwareData = new uint8_t[contentLength];
    size_t firmwareSize = contentLength;
    size_t firmwareDataIndex = 0;
    auto stream = http.getStreamPtr();
    uint32_t crc = 0;
    while (http.connected() && (contentLength > 0 || contentLength == -1)) {
        // Получаем данные и записываем их в массив firmwareData
        size_t len = stream->available();
        if (len) {
            uint8_t buffer[128];
            int readLen = stream->readBytes(buffer, ((len > sizeof(buffer)) ? sizeof(buffer) : len));
            memcpy(firmwareData + firmwareDataIndex, buffer, readLen);
            firmwareDataIndex += readLen;

            crc = crc32(crc, buffer, readLen);

            if (contentLength != -1) {
                contentLength -= readLen;
            }
        }
        delay(10);
    }

    // Преобразование контрольной суммы в строку
    char crcBuffer[9];
    sprintf(crcBuffer, "%08x", crc);
    String calculatedCRC = String(crcBuffer);
    // Завершение работы с HTTP клиентом
    http.end();

    if (calculatedCRC.equalsIgnoreCase(fileCRC)) {
        sendCheckFwRsp(0x01);
        _firmwareData = firmwareData;
        _firmwareSize = firmwareSize;
    } else {
        sendCheckFwRsp(0x02);
    }

    // Освобождение памяти, выделенной для массива firmwareData
    delete[] firmwareData;
    firmwareSize = 0;
}

/**
 * @brief Отправляет ответ на команду проверки версии прошивки.
 * 
 * Функция отправляет ответ на запрос проверки версии прошивки с указанным результатом.
 * Результат может быть следующим: 0x01 (новая прошивка доступна), 0x02 (обновление прошивки не требуется),
 * или 0x03 (ошибка при получении обновления прошивки).
 *
 * @param result Результат проверки версии прошивки.
 */
void ESP32_STM_UART::sendCheckFwRsp(uint8_t result) {
  uint8_t response[8] = {0x01, result, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  _serial.write(response, sizeof(response));
}

/**
 * @brief Вычисляет контрольную сумму CRC32 для данных.
 * 
 * Функция вычисляет контрольную сумму CRC32 для переданного массива данных размером dataSize.
 *
 * @param data Указатель на массив данных для вычисления CRC32.
 * @param dataSize Размер массива данных.
 * @return Строка, содержащая вычисленную CRC32 в шестнадцатеричном формате.
 */
String ESP32_STM_UART::calculateCRC32(uint8_t *data, uint32_t dataSize) {
  uint32_t crc = crc32(0L, Z_NULL, 0); // Инициализация CRC32
  crc = crc32(crc, reinterpret_cast<const Bytef *>(data), dataSize);
  char crcBuffer[9];
  sprintf(crcBuffer, "%08x", crc);
  return String(crcBuffer);
}

//Функции ниже определены для сбора проекта без ошибок.
//Буду постепенно их закидывать
void ESP32_STM_UART::sendFirmwareToSTM(const uint8_t *firmwareData, size_t firmwareSize){

}

void ESP32_STM_UART::sendWeightRsp(uint8_t result){

}

uint32_t ESP32_STM_UART::getUnixTimestamp(){
 return 0;
}

void ESP32_STM_UART::sendTimestampRsp(uint32_t timestamp, uint8_t result){

}

bool ESP32_STM_UART::sendDataToServer(uint8_t *cowId, uint32_t ts, float *weight, uint8_t weightCount){
 return false;
}

void ESP32_STM_UART::sendWeightCmdTransmitPrepare(){

}