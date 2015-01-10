#include "Arduino.h"
#include "relayboard.h"
#include "setup.h"
#include "bv4627.h"

void RelayBoard::init()
{
    _board = new BV4627(_address);
    _board->reset();
}

RelayBoard::RelayBoard(uint8_t id, uint8_t address) : Device(id, NO_NVSLOT, RELAYBOARD)
{
    _address = address;
    init();
}

RelayBoard::RelayBoard(uint8_t id, unsigned char *initData): Device(id,initData)
{
    _address = initData[_offset++];
    init();
}

RelayBoard::~RelayBoard()
{
    _board->off();
    delete _board;
}

void RelayBoard::setAddress(uint8_t address)
{
    Serial.print(F("Setting relayboard address to "));
    Serial.print(address,DEC);
    Serial.print(F(" (0x"));
    Serial.print(address,HEX);
    Serial.println(F(")"));

    _board->setaddress(address<<1);
    _address = address;
}


uint8_t RelayBoard::saveConfig(unsigned char *initData)
{
    uint8_t offset = Device::saveConfig(initData);
    initData[offset++]=_address;
    return offset;
}

void RelayBoard::setOn(uint8_t relay,uint8_t state)
{
    status1(1);
    _board->click(relay, state, 1);
    status1(0);
}

void RelayBoard::delayedSetOn(uint8_t relay,uint8_t state, int seconds)
{
    status1(1);
    _board->click(relay, state, 1 + seconds * 13);
    status1(0);
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
    Serial.print(F("\tDevice ID "));
    Serial.println(_board->deviceid());
    // firmware version
    *s=0;
    _board->version(s);
    Serial.print(F("\tFirmware Version "));
    Serial.println(s);
}

void RelayBoard::printDefinition(uint8_t first)
{
    {
        char buffer[40];
        sprintf(buffer,"define relayboard %d %d",getId(), _address);
        Serial.println(buffer);
    }
    Device::printDefinition(0);
};

bool RelayBoard::isType(enum DeviceType type)
{
    if(type == RELAYBOARD)
        return true;
    else
        return Device::isType(type);
}
