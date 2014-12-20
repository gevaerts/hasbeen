#include "Arduino.h"
#include "follower.h"
#include "setup.h"

void Follower::init()
{
    _waitingForOn = 0;
    _waitingForOff = 0;
}

Follower::Follower(uint8_t id, uint8_t board, uint8_t relay, uint8_t master, uint16_t delayOn, uint16_t delayOff):Relay(id, NO_NVSLOT, board, relay, FOLLOWER)
{
    _master = master;
    _delayOn = delayOn;
    _delayOff = delayOff;
    init();
}

Follower::Follower(uint8_t id, unsigned char *initData): Relay(id,initData)
{
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
    Serial1.print(F("\tmaster # "));
    Serial1.println(_master);
    Serial1.print(F("\tdelayOn "));
    Serial1.println(_delayOn);
    Serial1.print(F("\tdelayOff "));
    Serial1.println(_delayOff);
}

void Follower::printDefinition(uint8_t first)
{
    {
        char buffer[40];
        sprintf(buffer,"define follower %d %d %d %d %d %d",getId(), getBoard(), getRelay(), _master, _delayOn, _delayOff);
        Serial1.println(buffer);
    }
    Device::printDefinition(0);
};

void Follower::notify(uint8_t device, bool on)
{
    if(device == _master)
    {
        if(on)
        {
            _onRequest = millis();
            _waitingForOn = 1;
            _waitingForOff = 0;
        }
        else
        {
            _offRequest = millis();
            _waitingForOff = 1;
            _waitingForOn = 0;
        }
    }
}

void Follower::loop()
{
    if(_waitingForOn)
    {
        if((millis() - _onRequest)/1000 >= _delayOn)
        {
            _waitingForOn = 0;
            setOn(1);
        }
    }
    if(_waitingForOff)
    {
        if((millis() - _offRequest)/1000 >= _delayOff)
        {
            _waitingForOff = 0;
            setOn(0);
        }
    }
    Relay::loop();
}

bool Follower::isType(enum DeviceType type)
{
    if(type == FOLLOWER)
        return true;
    else
        return Relay::isType(type);
}
