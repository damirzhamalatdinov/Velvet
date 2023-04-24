/**
 * @file Settings_ESP.h
 * @brief Файл содержит внешние объявления функции отладки, SSID и пароля Wi-Fi.
 */
#ifndef Settings_ESP_h
#define Settings_ESP_h

#pragma once

/**
 * @brief SSID Wi-Fi сети, к которой должен подключаться ESP32.
 */
extern const char* ssid;
/**
 * @brief Пароль Wi-Fi сети, к которой должен подключаться ESP32.
 */
extern const char* password;

/**
 * @brief Логическое значение, определяющее, следует ли печатать входную переменную.
 */
extern bool debug_flag;

template <typename T>

/**
 * @brief Функция отладки, которая печатает входную переменную, если флаг равен true. Состояние флага устанавливается в main.cpp но видно во всём проекте.
 *
 * Эта шаблонная функция принимает логический флаг и переменную любого типа. Он печатает значение переменной
 * использование Serial.println(), если флаг установлен. Если флаг ложный, функция ничего не делает.
 * 
 * @tparam T Тип входной переменной для печати.
 * @param flag Логическое значение, определяющее, следует ли печатать входную переменную.
 * @param variable Переменная, которая будет напечатана, если флаг истинен.
 *
 * Пример использования:
 * @code
 *   int number = 42;
 *   bool debug_flag = true;
 *   print_debug(debug_flag, number);
 * @endcode
 */
void print_debug(bool flag, const T& variable) {
  if (flag) {
    Serial.println(variable);
  }
}

#endif