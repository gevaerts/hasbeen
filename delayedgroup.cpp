#include "Arduino.h"
#include "delayedgroup.h"
#include "setup.h"

void DelayedGroup::init()
{
    _waitingForOn = 0;
    _waitingForOff = 0;
}

DelayedGroup::DelayedGroup(uint8_t id, uint8_t master, uint16_t delayOn, uint16_t delayOff):Device(id, NO_NVSLOT, DELAYEDGROUP)
{
    _master = master;
    _delayOn = delayOn;
    _delayOff = delayOff;
    init();
}

DelayedGroup::DelayedGroup(uint8_t id, unsigned char *initData): Device(id,initData)
{
    _master = initData[_offset++];
    _delayOn = initData[_offset++]<<8;
    _delayOn += initData[_offset++];
    _delayOff = initData[_offset++]<<8;
    _delayOff += initData[_offset++];
    memcpy(_memberBitmap,&initData[_offset],sizeof(_memberBitmap));
    _offset+=sizeof(_memberBitmap);
    init();
}

DelayedGroup::~DelayedGroup()
{
}

uint8_t DelayedGroup::saveConfig(unsigned char *initData)
{
    uint8_t offset = Device::saveConfig(initData);
    initData[offset++]=_master;
    initData[offset++]=_delayOn>>8;
    initData[offset++]=_delayOn&0xff;
    initData[offset++]=_delayOff>>8;
    initData[offset++]=_delayOff&0xff;
    memcpy(&initData[offset],_memberBitmap,sizeof(_memberBitmap));
    offset+=sizeof(_memberBitmap);
    return offset;
}

void DelayedGroup::printInfo()
{
    Device::printInfo();
    Serial.print(F("\tmaster # "));
    Serial.println(_master);
    Serial.print(F("\tdelayOn "));
    Serial.println(_delayOn);
    Serial.print(F("\tdelayOff "));
    Serial.println(_delayOff);
    Serial.print(F("\tmembers "));
    for(int i=0;i<sizeof(_memberBitmap)*8;i++)
    {
        if(isMember(i))
        {
            Serial.print(i);
            Serial.print(F(" "));
        }
    }
    Serial.println();
}


void DelayedGroup::addMember(uint8_t id)
{
    uint8_t mbyte = id / 8;
    uint8_t mbit = id % 8;
    if(mbyte < sizeof(_memberBitmap))
    {
        _memberBitmap[mbyte] |= (1<<mbit);
    }
}

void DelayedGroup::removeMember(uint8_t id)
{
    uint8_t mbyte = id / 8;
    uint8_t mbit = id % 8;
    if(mbyte < sizeof(_memberBitmap))
    {
        _memberBitmap[mbyte] &= ~(1<<mbit);
    }
}

bool DelayedGroup::isMember(uint8_t id)
{
    uint8_t mbyte = id / 8;
    uint8_t mbit = id % 8;
    if(mbyte < sizeof(_memberBitmap))
    {
        return (_memberBitmap[mbyte] & (1<<mbit)) != 0;
    }
    else
    {
        return false;
    }
}

void DelayedGroup::on()
{
    for(int i=0;i < NUM_DEVICES;i++)
    {
        if(isMember(i))
        {
            Device *d = getDeviceForId(i);
            if(d!=NULL)
                d->on();
        }
    }
    Device::on();
}

void DelayedGroup::off()
{
    for(int i=0;i < NUM_DEVICES;i++)
    {
        if(isMember(i))
        {
            Device *d = getDeviceForId(i);
            if(d!=NULL)
                d->off();
        }
    }
    Device::off();
}


void DelayedGroup::printDefinition(uint8_t first)
{
    {
        char buffer[40];
        sprintf(buffer,"define delayedgroup %d %d %d %d",getId(), _master, _delayOn, _delayOff);
        Serial.println(buffer);
        for(int i=0;i<sizeof(_memberBitmap)*8;i++)
        {
            if(isMember(i))
            {
                sprintf(buffer,"delayedgroup add %d %d",getId(), i);
                Serial.println(buffer);
            }
        }
    }
    Device::printDefinition(0);
};

void DelayedGroup::notify(uint8_t device, bool on)
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

void DelayedGroup::loop()
{
    if(_waitingForOn)
    {
        if((millis() - _onRequest)/1000 >= _delayOn)
        {
            _waitingForOn = 0;
            on();
        }
    }
    if(_waitingForOff)
    {
        if((millis() - _offRequest)/1000 >= _delayOff)
        {
            _waitingForOff = 0;
            off();
        }
    }
    Device::loop();
}

bool DelayedGroup::isType(enum DeviceType type)
{
    if(type == DELAYEDGROUP)
        return true;
    else
        return Device::isType(type);
}
