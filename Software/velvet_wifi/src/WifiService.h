#include "config.h"
#include "WiFi.h"


class WifiService
{
private:
    char ssid[50] = WIFI_SSID;
    char password[50] = WIFI_PASSWORD;
    char fwVersion[10] = FW_VERSION;
    bool status = false;
    /* data */
public:
    WifiService(/* args */);
    ~WifiService();
    void begin();
    wl_status_t getStatus();
};


