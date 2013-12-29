#ifndef NVRAM_H
#define NVRAM_H

#include "Arduino.h"

class NVRam
{
    public:
        NVRam()
        {
        }
        ~NVRam()
        {
        }
        void getRAM(uint8_t rtc_addr, uint8_t * rtc_ram, uint8_t rtc_quantity);
        void setRAM(uint8_t rtc_addr, uint8_t * rtc_ram, uint8_t rtc_quantity);
    protected:
    private:
};

extern NVRam NVRAM;

#endif

