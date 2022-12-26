#include "Stm.h"
#include <HTTPClient.h>


Stm::Stm(/* args */)
{
  Serial2.begin(115200);    // debug serial
}

Stm::~Stm()
{
}

bool Stm::checkUpdate(){
    HTTPClient http;
    http.setTimeout(10000);
    http.begin(serverPath);
    if (http.GET() == HTTP_CODE_OK){
      isUpdate = true;
    } else {
      isUpdate = false;
    }
    http.end();
    return isUpdate;
}

void Stm::sendUpdateStatus(){
  updateValData[2] = isUpdate;
  updateValData[3] = (updateValData[0] + updateValData[1] + updateValData[2]) & 0xff;
  Serial2.flush();
  Serial2.write(updateValData, 4);
  Serial2.flush();
}

void getSizeCmd(uint16_t size, uint8_t *data){
  data[0] = 5;
  data[1] = (size & 0xff0000) >> 16;;
  data[2] = (size & 0xff00) >> 8;
  data[3] = size & 0xff;
  data[4] = (data[0] + data[1] + data[2] + data[3]) & 0xff;
}

bool Stm::downloadFw(){
  if (isUpdate){
    unsigned long start = millis();
    HTTPClient http;
    http.begin(serverPath);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      // get lenght of document (is -1 when Server sends no Content-Length header)
      int len = http.getSize();
      uint16_t constLen = len;
      Serial.printf("LEN: %d\n", len);
      char buff[5] = { 0 };
      getSizeCmd(constLen + 1, updateSize);
      
      

      Serial2.write(updateSize, sizeof(updateSize));
      Serial2.flush();

      WiFiClient * stream = http.getStreamPtr();

      int a = 0, sum = 0;
      uint8_t checksum = 0;
      uint8_t tempBuf[10];

      while (http.connected() && (len > 0 || len == -1)) {
        // get available data size
        size_t size = stream->available();
        
        if (size) {
          // int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          uint8_t ss = Serial2.read();;
          
          if (ss != 0x74){
            continue;
          }
          int c = stream->readBytes(buff, 4);
          sum += c;
          
          checksum = 0;
          for (int j = 0; j < c; j++)
            checksum +=  buff[j];
          buff[4] = checksum;

          Serial2.write(buff, c+1);
          Serial2.flush();

          // Serial.print("[");
          // for (int i = 0; i < 5; i++){
          //   Serial.printf("%X, ", buff[i]);
          // }
          // Serial.printf("] - ");


          Serial.printf("%d - %d/%d  %.1f\% \n", a, constLen, sum, ((float)sum/ (float)constLen)*100);
          if (len > 0) {
            len -= c;
          }
        // Serial.printf("AVAILABLE SIZE: %d\n", size);
        // Serial.printf("  BUFSIZE SIZE: %d\n", sizeof(buff));
        // Serial.printf("READBYTES SIZE: %d\n", c);
        
        }
        a += 1;
        // delay(5);
      }
      Serial.println();
      // file.close();
      Serial.print("[HTTP] connection closed or file end.\n");
      return 1;
    } 
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
  }
  return 0;
}