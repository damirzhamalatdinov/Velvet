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
#include <WiFiClientSecure.h>
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

void ESP32_STM_UART::sendcmd(uint8_t result) {
  uint8_t response[8] = {0x01, result, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  _serial.write(response, sizeof(response));
}


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
    std::vector<uint8_t> receivedBytes; // Чтение доступных байтов из буфера и сохранение их в векторе
    while (_serial.available()) {
        receivedBytes.push_back(_serial.read());
    }
    if (receivedBytes.empty()) {    // Проверка длины сообщения, если пустое выходим
        return;
    }
    uint8_t cmd = receivedBytes[0]; // Использование первого байта сообщения в качестве команды
    print_debug(debug_flag, String("Received command: 0x") + String(cmd, HEX));

    switch (cmd) {
        case CHECK_FW_CMD: {
            uint8_t version0 = receivedBytes[1];
            uint8_t version1 = receivedBytes[2];
            checkFirmware(version0, version1);
            break;
        }
        case BOOTLOADER_READY_CMD: {
            sendFirmwareToSTM(_firmwareData, _firmwareSize);
            break;
        }
        case SEND_WEIGHT_CMD_TRANSMIT_PREPARE: {
            sendWeightCmdTransmitPrepare();
            break;
        }
        case GET_TIMESTAMP_CMD: {
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

void ESP32_STM_UART::checkFirmware(uint8_t version0, uint8_t version1) {
    if (!check_wifi()){
        sendCheckFwRsp(0x02);
        return;
    }
    String version = String(version0) + "." + String(version1);
    //String serialNumber = String(ESP.getChipRevision());
    // Получение обновления прошивки с сервера
    WiFiClientSecure wifiClient;
    HTTPClient http;
    String url = "https://smart-farm.kz:8502/api/v2/ActualScalesFirmware";
    String postData = "serialNumber=" + serialNumber + "&firmwareVersion=" + version;
    print_debug(debug_flag, String("SerialNumber " + String(serialNumber) + "Version" + String(version)));
    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/json; charset=utf-8");
    int httpResponseCode = http.POST(postData);
    print_debug(debug_flag, String("Status_code: " + String(httpResponseCode)));
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
    print_debug(debug_flag, String("Firmware downloaded. Size:" + String(firmwareSize)));
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
        print_debug(debug_flag, String("Everything OK"));
    } else {
        sendCheckFwRsp(0x02);
        print_debug(debug_flag, String("Error: Wrong CRC"));
    }

    // Освобождение памяти, выделенной для массива firmwareData
    delete[] firmwareData;
    //firmwareSize = 0;
}

void ESP32_STM_UART::sendCheckFwRsp(uint8_t result) {
  uint8_t response[8] = {0x01, result, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  _serial.write(response, sizeof(response));
}

void ESP32_STM_UART::sendFirmwareToSTM(const uint8_t *firmwareData, size_t firmwareSize) {

    print_debug(debug_flag, String("Sending firmware to STM..."));
    // Отправляем ответ по UART {0x02, 2B Size of FW, 0x00, 0x00, 0x00, 0x00, 0x00} - Bootloader ready RSP, Size of FW
    uint8_t bootloaderReadyRsp[8] = {0x02, (uint8_t)(firmwareSize & 0xFF), (uint8_t)((firmwareSize >> 8) & 0xFF), 0x00, 0x00, 0x00, 0x00, 0x00};
    _serial.write(bootloaderReadyRsp, sizeof(bootloaderReadyRsp));

    print_debug(debug_flag, String("Sent Bootloader Ready Response, waiting for command..."));
    // Ждем команду {0x03, 0x01, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00} - Ready to receive data CMD с timeout = 60 секунд
    uint32_t startTime = millis();
    bool cmdReceived = false;
    while (millis() - startTime < 60000) {
        if (_serial.available() >= 8) {
            uint8_t cmd[8];
            _serial.readBytes(cmd, 8);
            if (cmd[0] == 0x03 && cmd[1] == 0x01) {
                cmdReceived = true;
                break;
            }
        }
    }
    // Если команда не была получена, выходим из функции
    if (!cmdReceived) {
        print_debug(debug_flag, String("No command received, exiting..."));
        return;
    }
    print_debug(debug_flag, String("Command received, sending firmware..."));

    // Если команда пришла, отправляем файл прошивки по UART в по 256 байт {0x03, 256B FW Data} - Transmit data RSP
    size_t bytesSent = 0;
    while (bytesSent < firmwareSize) {
        uint8_t transmitDataRsp[257] = {0x03};
        size_t chunkSize = min((size_t)256, firmwareSize - bytesSent);
        memcpy(&transmitDataRsp[1], &firmwareData[bytesSent], chunkSize);
        _serial.write(transmitDataRsp, chunkSize + 1);
        bytesSent += chunkSize;
    }
    print_debug(debug_flag, String("Firmware sent, cleaning up..."));
    delete[] _firmwareData;
    _firmwareData = nullptr;
    _firmwareSize = 0;
    print_debug(debug_flag, String("Done."));
}

void ESP32_STM_UART::sendWeightCmdTransmitPrepare() {
    print_debug(debug_flag, String("Preparing to send weight command..."));

    if (!check_wifi()){
        print_debug(debug_flag, String("Wi-Fi check failed. Sending response..."));
        sendWeightRsp(0x02);
        return;
    }

    // Отправить ответ об успешной подготовке передачи данных
    uint8_t transmit_prepare_ok[8] = {0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Отправляем сформированный ответ по UART
    _serial.write(transmit_prepare_ok, sizeof(transmit_prepare_ok));
    print_debug(debug_flag, String("Sent successful data transmit preparation response."));

    // Ждем команду от STM с timeout 70 секунд
    uint32_t startTime = millis();
    while (millis() - startTime < 70000) {
        if (_serial.available()) {
            uint8_t cmd = _serial.read();
            if (cmd == SEND_WEIGHT_CMD_TRANSMIT_WEIGHT_ARRAY) {
                // Читаем данные из UART
                uint8_t cowId[6];
                uint32_t ts;
                uint32_t weightCount = 60;
                float weight[weightCount];

                _serial.readBytes(cowId, 6);
                _serial.readBytes((uint8_t *)&ts, 4);
                for (size_t i = 0; i < weightCount; i++) {
                    _serial.readBytes((uint8_t *)&weight[i], 4);
                }

                print_debug(debug_flag, String("Data read from UART. Sending to server..."));

                // Отправить данные на сервер
                bool result = sendDataToServer(cowId, ts, weight, weightCount);

                // Отправить результат операции на STM
                sendWeightRsp(result ? 0x01 : 0x02);
                print_debug(debug_flag, String("Result sent to STM: ") + String(result ? "0x01" : "0x02"));
                return;
            }
        }
    }
    print_debug(debug_flag, String("Timeout reached without receiving weight data."));
}

// ???
bool ESP32_STM_UART::sendDataToServer(uint8_t *cowId, uint32_t ts, float *weight, uint8_t weightCount) {

    // При вызове функции мы должны сформировать следующую json строку 
    // {
    // "ScalesSerialNumber": "<serialNumber>",
    // "WeighingStart": "<ts - 60>",
    // "WeighingEnd": "<ts>",
    // "RFIDNumber": "<cowId>",
    // "Data": [<weight1>, <weight2>, ..., <weightN>]
    // }

    HTTPClient http;
    String url = "https://smart-farm.kz:8502/v2/OneTimeWeighings";
    http.begin(url);
    http.addHeader("Content-Type", "application/json; charset=utf-8");
    char tsStr[11]; //??
    sprintf(tsStr, "%u", ts); //??
    // Формирование JSON объекта
    String jsonPayload = "{\"ScalesSerialNumber\": \"" + serialNumber + "\",\"WeighingStart\":";
    jsonPayload += String(ts - 60);
    jsonPayload += ",\"WeighingEnd\":";
    jsonPayload += tsStr;
    jsonPayload += ",\"RFIDNumber\":\"";

    char cowIdStr[2 * sizeof(6) + 1];
    for (int i = 0; i < sizeof(6); i++) {
        sprintf(&cowIdStr[i * 2], "%02X", cowId[i]); // ??? Ошибка вместо 6 байт, записывает 4
    }
    jsonPayload += cowIdStr;
    jsonPayload += "\",\"Data\":[";
    for (int i = 0; i < weightCount; i++) {
        char weightStr[8];
        sprintf(weightStr, "%.2f", weight[i]);
        jsonPayload += weightStr;
        if (i < weightCount - 1) {
            jsonPayload += ",";
        }
    }
    jsonPayload += "]}";
    // Просмотр сформированного json объекта
    print_debug(debug_flag, String("JSON Payload: ") + jsonPayload);
    // Отправка POST запроса
    int httpResponseCode = http.POST(jsonPayload);
    // Просмотр STATUS_CODE
    print_debug(debug_flag, String("HTTP Response Code: ") + httpResponseCode);
    // Завершение работы с HTTP клиентом
    http.end();
    // Возвращаем результат отправки данных
    return httpResponseCode == 200;
}


void ESP32_STM_UART::sendWeightRsp(uint8_t result) {
    uint8_t response[8] = {0x05, result, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // Заполняем оставшиеся байты сообщения нулями
    for (int i = 2; i < 8; i++) {
        response[i] = 0x00;
    }
    // Отправляем сформированный ответ по UART
    _serial.write(response, sizeof(response));

    // Добавляем вывод отладочной информации
    String debug_msg = "Sending weight response: ";
    for(int i = 0; i < sizeof(response); i++) {
        debug_msg += String(response[i], HEX) + " ";
    }
    print_debug(debug_flag, debug_msg);
}

// Добавить UTC+6 
uint32_t ESP32_STM_UART::getUnixTimestamp() {
    bool wi_fi = check_wifi();
    if (!wi_fi){
        print_debug(debug_flag, "WiFi is not connected. Cannot get Unix timestamp.");
        return 0;
    }
    configTime(6 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    time_t now;
    uint8_t timeout = 30;
    time(&now);
    while (now < 1510644967 && timeout > 0) {
        delay(1000);
        time(&now);
        timeout--;
    }
    if (timeout == 0) {
        print_debug(debug_flag, "NTP request timed out. Cannot get Unix timestamp.");
        return 0;
    }
    print_debug(debug_flag, "Unix timestamp is: " + String(now));
    return now;
}


void ESP32_STM_UART::sendTimestampRsp(uint32_t timestamp, uint8_t result) {
    // ESP32 -> STM Ответ на с числом unix
    uint8_t response[8];
    response[0] = 0x06; // GET_TIMESTAMP_RSP
    response[1] = result;
    if (result == 0x00) {
        response[2] = (timestamp >> 24) & 0xFF;
        response[3] = (timestamp >> 16) & 0xFF;
        response[4] = (timestamp >> 8) & 0xFF;
        response[5] = timestamp & 0xFF;
        print_debug(debug_flag, String("Timestamp: ") + String(timestamp));
    } else {
        for (int i = 2; i < 6; i++) {
        response[i] = 0x00;
        }
        print_debug(debug_flag, String("Timestamp not available. Result: ") + String(result));
    }

    response[6] = 0x00;
    response[7] = 0x00;

    _serial.write(response, sizeof(response));
    print_debug(debug_flag, String("Timestamp response sent."));
} 
