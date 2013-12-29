#include "Arduino.h"
#include "relayboard.h"
#include "setup.h"
#include <bv4627.h>
#include <Wire.h>

void RelayBoard::init()
{
    _board = new BV4627(_address);
}

RelayBoard::RelayBoard(uint8_t id, uint8_t address) : Device(id, NO_NVSLOT, RELAYBOARD)
{
    _address = address;
    init();
}

RelayBoard::RelayBoard(uint8_t id, unsigned char *initData): Device(id,initData)
{
    Serial.println(F("Restoring relayboard data"));
    _address = initData[_offset++];
    init();
}

RelayBoard::~RelayBoard()
{
    Serial.println(F("Deleting relayboard"));
    _board->off();
    delete _board;
}

uint8_t RelayBoard::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving relayboard data"));

    uint8_t offset = Device::saveConfig(initData);
    initData[offset++]=_address;
    return offset;
}

void RelayBoard::setOn(uint8_t relay,uint8_t state)
{
    _board->click(relay, state, 1);
}

void RelayBoard::printInfo()
{
    Device::printInfo();
    Serial.print(F("\taddress 0x"));
    Serial.println(_address,HEX);

    uint16_t k=1;
    char s[10]; 
    // device id
    Serial.println();
    Serial.print("\tDevice ID ");
    Serial.println(_board->deviceid());
    // firmware version
    *s=0;
    _board->version(s);
    Serial.print("\tFirmware Version ");
    Serial.println(s);
}
