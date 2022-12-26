
#include "Stm.h"
#include "WifiService.h"

WifiService stmWifi;
Stm stm;


bool validData(uint8_t *data, size_t size);

void setup() {
  Serial.begin(115200);    // debug serial
  delay(500);
  
  stmWifi.begin();
  if (stmWifi.getStatus() != WL_CONNECTED){
    Serial.println("ESP RESTART...");
    ESP.restart();
  }
}


uint8_t uartReadBuf[50];


void loop() {
  if(Serial2.available()){
    uint8_t size = Serial2.read();
    uint8_t c = (size > sizeof(uartReadBuf)) ? sizeof(uartReadBuf) : size;
    Serial2.readBytes(uartReadBuf, c);
    if (validData(uartReadBuf, c)){
      Serial.printf("CASE %d\n", uartReadBuf[0]);
      switch (uartReadBuf[0])
      {
      case 1:
        stm.checkUpdate();
        stm.sendUpdateStatus();
        break;
      case 2:
        stm.downloadFw();
        break;
      
      default:
        Serial.println("DEFAULT");
        break;
      }
    }
    
    // Serial.print("[");
    // for (int i = 0; i < size; i++){
    //   Serial.printf("%X, ", uartReadBuf[i]);
    // }
    // Serial.printf("]\n");
  }
  delay(5);
}


bool validData(uint8_t *data, size_t size){
  uint8_t sum = size;
  int i = 0;
  for (;i < size-1;i++){
    sum += data[i];
  }
  if (sum == data[size - 1])
    return true;
  return false;
}






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