#include "nvram.h"
#include <Wire.h>

#define ADDRESS 0x68

void NVRam::getRAM(uint8_t rtc_addr, uint8_t * rtc_ram, uint8_t rtc_quantity)
{
  Wire.beginTransmission(ADDRESS);
  rtc_addr &= 63;                       // avoid wrong adressing. Adress 0x08 is now address 0x00...
  rtc_addr += 8;                        // ... and address 0x3f is now 0x38
  Wire.write(rtc_addr);                  // set CTRL Register Address
  if ( Wire.endTransmission() != 0 )
    return;
  Wire.requestFrom(ADDRESS, (int)rtc_quantity);
  while(!Wire.available())
  {
    // waiting
  }
  for(uint8_t i=0; i<rtc_quantity; i++)     // Read x data from given address upwards...
  {
    rtc_ram[i] = Wire.read();        // ... and store it in rtc_ram
  }
}

void NVRam::setRAM(uint8_t rtc_addr, uint8_t * rtc_ram, uint8_t rtc_quantity)
{
  Wire.beginTransmission(ADDRESS);
  rtc_addr &= 63;                       // avoid wrong adressing. Adress 0x08 is now address 0x00...
  rtc_addr += 8;                        // ... and address 0x3f is now 0x38
  Wire.write(rtc_addr);                  // set RAM start Address 
  for(uint8_t i=0; i<rtc_quantity; i++)     // Send x data from given address upwards...
  {
    Wire.write(rtc_ram[i]);              // ... and send it from rtc_ram to the RTC Chip
  }
  Wire.endTransmission();
}

NVRam NVRAM = NVRam();
