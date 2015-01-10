#include "nvram.h"
#include <I2C.h>

#define ADDRESS 0x68

void NVRam::getRAM(uint8_t rtc_addr, uint8_t * rtc_ram, uint8_t rtc_quantity)
{
    rtc_addr &= 63;                       // avoid wrong adressing. Adress 0x08 is now address 0x00...
    rtc_addr += 8;                        // ... and address 0x3f is now 0x38
    if ( I2c.write((uint8_t)ADDRESS, rtc_addr) != 0)
        return;

    I2c.read(ADDRESS, (int)rtc_quantity);
    for(uint8_t i=0; i<rtc_quantity; i++)     // Read x data from given address upwards...
    {
        rtc_ram[i] = I2c.receive();        // ... and store it in rtc_ram
    }
}

void NVRam::setRAM(uint8_t rtc_addr, uint8_t * rtc_ram, uint8_t rtc_quantity)
{
    rtc_addr &= 63;                       // avoid wrong adressing. Adress 0x08 is now address 0x00...
    rtc_addr += 8;                        // ... and address 0x3f is now 0x38
    I2c.write(ADDRESS, rtc_addr, rtc_ram, rtc_quantity);
}

NVRam NVRAM = NVRam();
