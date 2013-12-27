#include "Arduino.h"
#include "relayboard.h"
#include "setup.h"
#include <bv4627.h>
#include <Wire.h>

void RelayBoard::init()
{
    _board = new BV4627(_address);

}

RelayBoard::RelayBoard(int id, int address) : Device(id, RELAYBOARD)
{
    _address = address;
    init();
}

RelayBoard::RelayBoard(int id, unsigned char *initData): Device(id,initData)
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

int RelayBoard::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving relayboard data"));

    int offset = Device::saveConfig(initData);
    initData[offset++]=_address;
    return offset;
}

void RelayBoard::setOn(int relay,int state)
{
    _board->click(relay, state, 1);
}

void RelayBoard::printInfo()
{
    Device::printInfo();
    Serial.print(F("\taddress  "));
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
