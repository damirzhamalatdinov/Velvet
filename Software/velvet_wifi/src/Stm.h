#include "stdint.h"

class Stm
{
private:
    bool isUpdate = false;
    uint8_t updateValData[4] = {0x04, 0x01, 0x00, 0x00};
    uint8_t updateSize[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    char serverPath[100] = "http://172.31.7.161:5000/frimware/?token=tarcktor_token_01\0";
    
    /* data */
public:
    Stm(/* args */);
    ~Stm();
    bool checkUpdate();
    bool downloadFw();
    void sendUpdateStatus();
    void reboot();
    bool update();
};


