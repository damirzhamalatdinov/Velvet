#define BOOT_WIFI_SSID "OpenTechGroup"      // wifi ssid for frimware transfer
#define BOOT_WIFI_PASSWORD "korkemwifi"     // wifi password for frimware transfer
#define BOOT_TIMEOUT 30*1000               // 30000 milliseconds or 30 seconds
#define FW_VERSION "1.01"

#define WIFI_SSID "OpenTechGroup"
#define WIFI_PASSWORD "korkemwifi"

// unsigned long FW_CHECK_TIME = 2*60*1000;


// unsigned long RESTART_TIME = 2*60*60*1000;



// #include <WiFiMulti.h>
// #include <HTTPClient.h>
// #include "FS.h"
// #include "SPIFFS.h"
// #include <EEPROM.h>


// #define BOOT_WIFI_SSID "Blue_tractor"      // wifi ssid for frimware transfer
// #define BOOT_WIFI_PASSWORD "wifi_wifi"     // wifi password for frimware transfer
// #define BOOT_TIMEOUT 30000                 // timeout in milliseconds
// #define FORMAT_SPIFFS_IF_FAILED true
// #define ID_ADDRESS 10
// #define VERSION_ADDRESS 15


// WiFiMulti wifiMulti;
// const char* ssid = "OpenTechGroup";         // wifi ssid for data transfer
// const char* password = "korkemwifi";        // wifi password for data transfer
// String serverName = "http://172.28.61.211:5000/"; // frimware/  version/
// uint8_t id = 0;                             // ID of
// uint8_t current_version = 0;


// void setup() {
//   Serial.begin(115200);    // debug serial
//   delay(500);
//   Serial2.begin(115200);   // stm32 serial
//   delay(500);
//   EEPROM.begin(512);       // internel memory
//   delay(500);

//   if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {  //
//     Serial.println("SPIFFS Mount Failed");
//   }
//   id = EEPROM.read(ID_ADDRESS);
//   current_version = EEPROM.read(VERSION_ADDRESS);

//   Serial.println("-------------------------------------------------------");
//   Serial.println("Wifi start");
//   Serial.printf("ID: %i \t VERSION: %i\r\n", id, current_version);

//   wifiMulti.addAP(BOOT_WIFI_SSID, BOOT_WIFI_PASSWORD);
//   unsigned long boot_strart_time = millis();
//   while (1) {
//     if ((wifiMulti.run() == WL_CONNECTED)) {  // is connected to boot wifi
//       Serial.println("WIFI OK");
//       Serial.println("BOOT MODE");
//       int temp_version = getVersion();
//       if (temp_version != -1) {               // check is getVersion() succeed
//         if (current_version != temp_version) {
//           if (updateStm32()) {
//             Serial.println("UPDATE OK");
//             EEPROM.write(VERSION_ADDRESS, temp_version);
//             delay(100);
//             EEPROM.commit();
//             break;
//           } else {
//             Serial.println("UPDATE ERROR");
//             Serial.println("RESTART ESP32");
//             ESP.restart();
//           }
//         } else {
//           Serial.println("VERSION IS UP-TO-DATE");
//           break;
//         }
//       } else {
//         Serial.println("VERSION ERROR");
//         break;
//       }
//     }
//     if (millis() - boot_strart_time > BOOT_TIMEOUT) {  // timeout reached
//       Serial.println("WIFI ERROR, TIMEOUT");
//       Serial.printf("CAN'T CONNECT TO %s\r\n", BOOT_WIFI_SSID);
//       break;
//     }
//   }
//   delay(500);
//   //  wifiMulti.addAP(ssid, password);
// }

// void loop() {
// //  if (Serial2.available()) {
// //    while (Serial2.available()) {
// //      byte ch = (byte)Serial2.read();
// //      Serial.print(ch);
// //    }
// //    Serial.println();
// //  }



//   // wait for WiFi connection
//   //  Serial.println("Looop Start");
//   if ((wifiMulti.run() == WL_CONNECTED)) {

//     HTTPClient http;

//     Serial.print("[HTTP] begin...\n");
//     String serverPath = serverName + "?token=tarcktor_token_01\0";

//     // Your Domain name with URL path or IP address with path
//     http.begin(serverPath.c_str());

//     // configure server and url
//     // http.begin("http://192.168.1.12/test.html");
//     // serverName
//     //http.begin("192.168.1.12", 80, "/test.html");

//     Serial.print("[HTTP] GET...\n");
//     // start connection and send HTTP header
//     int httpCode = http.GET();
//     if (httpCode > 0) {
//       // HTTP header has been send and Server response header has been handled
//       Serial.printf("[HTTP] GET... code: %d\n", httpCode);

//       // file found at server
//       if (httpCode == HTTP_CODE_OK) {

//         // get lenght of document (is -1 when Server sends no Content-Length header)
//         int len = http.getSize();

//         // create buffer for read
//         char buff[2560] = { 0 };

//         // get tcp stream
//         WiFiClient * stream = http.getStreamPtr();

//         // read all data from server
//         int a = 0;

//         const char * path = "/test2.bin";
//         Serial.printf("Writing file: %s\r\n", path);
//         File file = SPIFFS.open(path, FILE_WRITE);

//         if (!file) {
//           Serial.println("- failed to open file for writing");
//         }


//         while (http.connected() && (len > 0 || len == -1)) {
//           // get available data size
//           size_t size = stream->available();

//           if (size) {
//             // read up to 128 byte
//             int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

//             // write it to Serial
//             // USE_SERIAL.write(buff, c);
//             // USE_SERIAL.println(a);
//             if (file.print(buff)) {
//               Serial.println("- file written");
//               Serial.println(a);
//             } else {
//               Serial.println("- write failed");
//               Serial.println(a);
//             }


//             if (len > 0) {
//               len -= c;
//             }
//           }
//           a += 1;
//           delay(1);
//         }

//         Serial.println();
//         file.close();
//         Serial.print("[HTTP] connection closed or file end.\n");

//       }
//     } else {
//       Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//     }

//     http.end();
//   }

//   delay(20000);
// }


// bool tryConnect(const char * ssid, const char * password, int timeout) {
//   wifiMulti.addAP(ssid, password);
//   Serial.println("WIFI OK");
//   unsigned long normal_strart_time = millis();
//   while (1) {
//     if ((wifiMulti.run() == WL_CONNECTED)) {  // is connected to boot wifi
//       Serial.println("WIFI OK");
//       Serial.println("BOOT MODE");
//       BOOT_MODE = true;
//       break;
//     }
//     if (millis() - boot_strart_time > BOOT_TIMEOUT) {  // timeout reached
//       Serial.println("WIFI ERROR, TIMEOUT");
//       Serial.println("CAN'T CONNECT TO" + BOOT_WIFI_SSID);
//       wifiMulti.addAP(ssid, password);
//       break;
//     }
//   }
// }



// void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
//   Serial.printf("Listing directory: %s\r\n", dirname);

//   File root = fs.open(dirname);
//   if (!root) {
//     Serial.println("- failed to open directory");
//     return;
//   }
//   if (!root.isDirectory()) {
//     Serial.println(" - not a directory");
//     return;
//   }

//   File file = root.openNextFile();
//   while (file) {
//     if (file.isDirectory()) {
//       Serial.print("  DIR : ");
//       Serial.println(file.name());
//       if (levels) {
//         listDir(fs, file.name(), levels - 1);
//       }
//     } else {
//       Serial.print("  FILE: ");
//       Serial.print(file.name());
//       Serial.print("\tSIZE: ");
//       Serial.println(file.size());
//     }
//     file = root.openNextFile();
//   }
// }

// void readFile(fs::FS &fs, const char * path) {
//   Serial.printf("Reading file: %s\r\n", path);

//   File file = fs.open(path);
//   if (!file || file.isDirectory()) {
//     Serial.println("- failed to open file for reading");
//     return;
//   }

//   Serial.println("- read from file:");
//   while (file.available()) {
//     Serial.write(file.read());
//   }
//   file.close();
// }

// void writeFile(fs::FS &fs, const char * path, const char * message) {
//   Serial.printf("Writing file: %s\r\n", path);

//   File file = fs.open(path, FILE_WRITE);
//   if (!file) {
//     Serial.println("- failed to open file for writing");
//     return;
//   }
//   if (file.print(message)) {
//     Serial.println("- file written");
//   } else {
//     Serial.println("- write failed");
//   }
//   file.close();
// }

// void appendFile(fs::FS &fs, const char * path, const char * message) {
//   Serial.printf("Appending to file: %s\r\n", path);

//   File file = fs.open(path, FILE_APPEND);
//   if (!file) {
//     Serial.println("- failed to open file for appending");
//     return;
//   }
//   if (file.print(message)) {
//     Serial.println("- message appended");
//   } else {
//     Serial.println("- append failed");
//   }
//   file.close();
// }

// void renameFile(fs::FS &fs, const char * path1, const char * path2) {
//   Serial.printf("Renaming file %s to %s\r\n", path1, path2);
//   if (fs.rename(path1, path2)) {
//     Serial.println("- file renamed");
//   } else {
//     Serial.println("- rename failed");
//   }
// }

// void deleteFile(fs::FS &fs, const char * path) {
//   Serial.printf("Deleting file: %s\r\n", path);
//   if (fs.remove(path)) {
//     Serial.println("- file deleted");
//   } else {
//     Serial.println("- delete failed");
//   }
// }

// void testFileIO(fs::FS &fs, const char * path) {
//   Serial.printf("Testing file I/O with %s\r\n", path);

//   static uint8_t buf[512];
//   size_t len = 0;
//   File file = fs.open(path, FILE_WRITE);
//   if (!file) {
//     Serial.println("- failed to open file for writing");
//     return;
//   }

//   size_t i;
//   Serial.print("- writing" );
//   uint32_t start = millis();
//   for (i = 0; i < 2048; i++) {
//     if ((i & 0x001F) == 0x001F) {
//       Serial.print(".");
//     }
//     file.write(buf, 512);
//   }
//   Serial.println("");
//   uint32_t end = millis() - start;
//   Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
//   file.close();

//   file = fs.open(path);
//   start = millis();
//   end = start;
//   i = 0;
//   if (file && !file.isDirectory()) {
//     len = file.size();
//     size_t flen = len;
//     start = millis();
//     Serial.print("- reading" );
//     while (len) {
//       size_t toRead = len;
//       if (toRead > 512) {
//         toRead = 512;
//       }
//       file.read(buf, toRead);
//       if ((i++ & 0x001F) == 0x001F) {
//         Serial.print(".");
//       }
//       len -= toRead;
//     }
//     Serial.println("");
//     end = millis() - start;
//     Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
//     file.close();
//   } else {
//     Serial.println("- failed to open file for reading");
//   }
// }
