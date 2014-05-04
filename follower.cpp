#include "Arduino.h"
#include "follower.h"
#include "setup.h"

void Follower::init()
{
}

Follower::Follower(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay, uint8_t master, uint16_t delayOn, uint16_t delayOff):Relay(id, nvSlot, board, relay, FOLLOWER)
{
    _master = master;
    _delayOn = delayOn;
    _delayOff = delayOff;
    init();
    saveState(0);
}

Follower::Follower(uint8_t id, unsigned char *initData): Relay(id,initData)
{
    Serial.println(F("Restoring lightpoint data"));
    _master = initData[_offset++];
    _delayOn = initData[_offset++]<<8;
    _delayOn += initData[_offset++];
    _delayOff = initData[_offset++]<<8;
    _delayOff += initData[_offset++];
    init();
}

Follower::~Follower()
{
}

uint8_t Follower::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving lightpoint data"));
    uint8_t offset = Relay::saveConfig(initData);
    initData[offset++]=_master;
    initData[offset++]=_delayOn>>8;
    initData[offset++]=_delayOn&0xff;
    initData[offset++]=_delayOff>>8;
    initData[offset++]=_delayOff&0xff;
    return offset;
}

void Follower::printInfo()
{
    Relay::printInfo();
    Serial.print(F("\tmaster # "));
    Serial.println(_master);
    Serial.print(F("\tdelayOn "));
    Serial.println(_delayOn);
    Serial.print(F("\tdelayOff "));
    Serial.println(_delayOff);
}

void Follower::printDefinition(uint8_t first)
{
    {
        char buffer[40];
        sprintf(buffer,"define follower %d %d %d %d %d %d %d",getId(), getNVSlot(), getBoard(), getRelay(), _master, _delayOn, _delayOff);
        Serial.println(buffer);
    }
    Device::printDefinition(0);
};

void Follower::notify(uint8_t device, bool on)
{
    // TODO implement delay
    if(device == _master)
    {
        setOn(on);
    }
}

bool Follower::isType(enum DeviceType type)
{
    if(type == FOLLOWER)
        return true;
    else
        return Relay::isType(type);
}
