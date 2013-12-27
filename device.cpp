#include <Arduino.h>
#include <EEPROM.h>
#include "device.h"
#include "setup.h"
#include "relay.h"
#include "relayboard.h"
#include "lightpoint.h"
#include "dimmer.h"

unsigned char Device::scratch[DEVICE_EEPROM_SIZE];
Device *Device::_devicesByButton[32]; //TODO: NUM_BUTTONS
Device *Device::_devicesById[32]; 

Device::Device(int id, enum DeviceType type)
{
    _id = id;
    _type = type;
    _name[0]=0;
    _devicesById[id]=this;
}

Device::Device(int id, unsigned char *initData)
{
    Serial.print(F("Restoring device data for device "));
    Serial.println(id);
    _offset = 0;
    _type = (enum DeviceType) initData[_offset++];
    _id = id;
    _offset++;
    memcpy(_name,initData+_offset,16);
    _offset += 16;
    _devicesById[id]=this;
}


Device::~Device()
{
    Serial.println(F("Deleting device"));
    _type = UNDEFINED;
    int baseAddress = _id * DEVICE_EEPROM_SIZE + DEVICE_EEPROM_BASE;
    scratch[0] = _type;
    EEPROM.write(baseAddress, scratch[0]);
    _devicesById[_id]=NULL;
}


int Device::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving device data"));

    int offset = 0;
    initData[offset++] = _type;
    initData[offset++] = _id;
    memcpy(&initData[offset+=16],_name,16);

    return offset;
}

void Device::saveSettings()
{
    saveConfig(scratch);
    int baseAddress = _id * DEVICE_EEPROM_SIZE + DEVICE_EEPROM_BASE;

    for(int i=0;i<DEVICE_EEPROM_SIZE;i++)
    {
        EEPROM.write(baseAddress + i, scratch[i]);
    }
}

void Device::printInfo()
{
    Serial.print(F("device id "));
    Serial.println(_id);
    Serial.print(F("\tdevice name "));
    Serial.println(_name);
    Serial.print(F("\tdevice type "));
    Serial.println(_type);
}

void Device::setName(char *name)
{
    strncpy(_name, name, 16);
    _name[15]=0;
}

void Device::restore(int id)
{
    Serial.println(F("Loading device data"));
    Device *device = NULL;
    int baseAddress = id * DEVICE_EEPROM_SIZE + DEVICE_EEPROM_BASE;
    enum DeviceType type = (enum DeviceType) EEPROM.read(baseAddress);

    if(type == UNDEFINED)
        return;

    scratch[0] = type;
    for(int i=1;i<DEVICE_EEPROM_SIZE;i++)
    {
        scratch[i]=EEPROM.read(baseAddress + i);
    }
    switch(type)
    {
        case DIMMEDLIGHT:
            device = new Dimmer(id,scratch);
            break;
        case ONOFFLIGHT:
            device = new Lightpoint(id,scratch);
            break;
        case RELAY:
            device = new Relay(id,scratch);
            break;
        case TIMEDRELAY:
            break;
        case RELAYBOARD:
            device = new RelayBoard(id,scratch);
            break;
        default:
            break;
    }
}

Device *Device::getDeviceForButton(int button)
{
    return _devicesByButton[button];
}

void Device::registerButton(int button, Device *device)
{
    _devicesByButton[button] = device;
}

Device *Device::getDeviceForId(int id)
{
    return _devicesById[id];
}

