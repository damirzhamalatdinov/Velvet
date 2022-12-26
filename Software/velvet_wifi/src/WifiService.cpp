#include "WifiService.h"

WifiService::WifiService(){

}

void WifiService::begin(/* args */)
{   
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 120000) {
        Serial.print('.');
        delay(700);
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED){
        Serial.printf("Wifi connected, local IP: %s\n", WiFi.localIP().toString());
    } else {
        Serial.printf("Can not connect to %s\n", ssid);
    }
}

wl_status_t WifiService::getStatus(){
    return WiFi.status();
}

WifiService::~WifiService()
{
}