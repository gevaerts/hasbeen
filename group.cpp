#include <Arduino.h>
#include "group.h"
#include "setup.h"

void Group::init()
{
    registerButton(_button, this);
}

Group::Group(uint8_t id, unsigned char *initData) : Device(id,initData)
{
    _button = initData[_offset++];
    memcpy(_memberBitmap,&initData[_offset],sizeof(_memberBitmap));
    _offset+=sizeof(_memberBitmap);
    init();
}


Group::Group(uint8_t id, uint8_t button) : Device(id, NO_NVSLOT, GROUP)
{
    memset(_memberBitmap, 0, sizeof(_memberBitmap));
    _button = button;
    memset(_memberBitmap,0,sizeof(_memberBitmap));
    init();
}

Group::~Group()
{
}

uint8_t Group::saveConfig(unsigned char *initData)
{
    uint8_t offset = Device::saveConfig(initData);
    initData[offset++] = _button;
    memcpy(&initData[offset],_memberBitmap,sizeof(_memberBitmap));
    offset+=sizeof(_memberBitmap);
    return offset;
}

void Group::printInfo()
{
    Device::printInfo();
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
    Serial.print(F("\tbutton # "));
    Serial.println(_button);
}

void Group::addMember(uint8_t id)
{
    uint8_t mbyte = id / 8;
    uint8_t mbit = id % 8;
    if(mbyte < sizeof(_memberBitmap))
    {
        _memberBitmap[mbyte] |= (1<<mbit);
    }
}

void Group::removeMember(uint8_t id)
{
    uint8_t mbyte = id / 8;
    uint8_t mbit = id % 8;
    if(mbyte < sizeof(_memberBitmap))
    {
        _memberBitmap[mbyte] &= ~(1<<mbit);
    }
}

bool Group::isMember(uint8_t id)
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

void Group::on()
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

void Group::off()
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

void Group::press(uint8_t button, uint8_t previousState)
{
    // If previousState was PRESSED, ignore this
    if((previousState == RELEASED) && button == _button)
    {
        // TODO: combination? Require multiple presses?
        off();
    }
}

void Group::printDefinition(uint8_t first)
{
    {
        char buffer[40];
        sprintf(buffer,"define group %d %d",getId(), _button);
        Serial.println(buffer);
        for(int i=0;i<sizeof(_memberBitmap)*8;i++)
        {
            if(isMember(i))
            {
                sprintf(buffer,"group add %d %d",getId(), i);
                Serial.println(buffer);
            }
        }
    }
    Device::printDefinition(0);
};

bool Group::isType(enum DeviceType type)
{
    if(type == GROUP)
        return true;
    else
        return Device::isType(type);
}
