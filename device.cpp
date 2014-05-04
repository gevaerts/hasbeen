#include <Arduino.h>
#include <EEPROM.h>
#include "device.h"
#include "setup.h"
#include "relay.h"
#include "relayboard.h"
#include "lightpoint.h"
#include "dimmer.h"
#include "group.h"
#include "nvram.h"

unsigned char Device::scratch[DEVICE_EEPROM_SIZE];
Device *Device::_devicesByButton[NUM_BUTTONS];
Device *Device::_devicesById[NUM_DEVICES];

Device::Device(uint8_t id, uint8_t nvSlot, enum DeviceType type)
{
    _id = id;
    _nvSlot = nvSlot;
    _type = type;
    _name[0]=0;
    _devicesById[id]=this;
}

Device::Device(uint8_t id, unsigned char *initData)
{
    Serial.print(F("Restoring device data for device "));
    Serial.println(id);
    _offset = 0;
    _type = (enum DeviceType) initData[_offset++];
    _id = id;
    _offset++;
    _nvSlot = initData[_offset++];
    memcpy(_name,initData+_offset,16);
    _offset += 16;
    _devicesById[id]=this;
}


Device::~Device()
{
    unregisterButtons(this);
    Serial.println(F("Deleting device"));
    _type = UNDEFINED;
    uint16_t baseAddress = _id * DEVICE_EEPROM_SIZE + DEVICE_EEPROM_BASE;
    scratch[0] = _type;
    EEPROM.write(baseAddress, scratch[0]);
    _devicesById[_id]=NULL;
}

uint8_t Device::restoreState()
{
    uint8_t data = 0;
    Serial.print(F("reading device state for device "));
    Serial.print(_id);
    Serial.print(F(" ... 0x"));
    if(_nvSlot != NO_NVSLOT)
    {
        NVRAM.getRAM(_nvSlot, &data, 1);
    }
    Serial.println(data,HEX);
    return data;
}
void Device::saveState(uint8_t data)
{
    Serial.print(F("saving device state : 0x"));
    Serial.println(data);
    if(_nvSlot != NO_NVSLOT)
    {
        NVRAM.setRAM(_nvSlot, &data, 1);
    }
}

uint8_t Device::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving device data"));

    uint8_t offset = 0;
    initData[offset++] = _type;
    initData[offset++] = _id;
    initData[offset++] = _nvSlot;
    memcpy(&initData[offset],_name,16);
    offset+=16;

    return offset;
}

void Device::saveSettings()
{
    uint8_t size = saveConfig(scratch);
    Serial.print(F("Saved "));
    Serial.print(size);
    Serial.print(F(" bytes for "));
    Serial.print(getTypeName());
    Serial.print(F(" "));
    Serial.println(getName());

    if(size > DEVICE_EEPROM_SIZE)
    {
        Serial.println(F("Error! Save size too large"));
    }
    uint16_t baseAddress = _id * DEVICE_EEPROM_SIZE + DEVICE_EEPROM_BASE;

    for(uint16_t i=0;i<DEVICE_EEPROM_SIZE;i++)
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
    Serial.println(getTypeName());
    Serial.print(F("\tNVRAM slot "));
    Serial.println(_nvSlot);
}

void Device::setName(char *name)
{
    strncpy(_name, name, 16);
    _name[15]=0;
}

void Device::restore(uint8_t id)
{
    Serial.println(F("Loading device data"));
    Device *device = NULL;
    uint16_t baseAddress = id * DEVICE_EEPROM_SIZE + DEVICE_EEPROM_BASE;
    enum DeviceType type = (enum DeviceType) EEPROM.read(baseAddress);

    if(type == UNDEFINED)
        return;

    scratch[0] = type;
    for(uint16_t i=1;i<DEVICE_EEPROM_SIZE;i++)
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
        case GROUP:
            device = new Group(id,scratch);
            break;
        default:
            break;
    }
    if(device != NULL)
    {
        device->restoreState();
    }
}

Device *Device::getDeviceForButton(uint8_t button)
{
    return _devicesByButton[button];
}

void Device::registerButton(uint8_t button, Device *device)
{
    if(button >= 0)
        _devicesByButton[button] = device;
}

void Device::unregisterButtons(Device *device)
{
    for(uint8_t i=0;i<NUM_BUTTONS;i++)
        if(_devicesByButton[i] == device)
            _devicesByButton[i] = NULL;
}

Device *Device::getDeviceForId(uint8_t id)
{
    if(id < NUM_DEVICES)
        return _devicesById[id];
    else
        return NULL;
}

void Device::printDefinition(uint8_t first)
{
    if(strlen(_name) > 0)
    {
        char buffer[40];
        sprintf(buffer,"setname %d %s",_id,_name);
        Serial.println(buffer);
    }
}

void Device::on()
{
    notifyAll(true);
}

void Device::off()
{
    notifyAll(false);
}

void Device::notifyAll(bool on)
{
    for(uint8_t i=0;i<NUM_DEVICES;i++)
    {
        Device *d = Device::getDeviceForId(i);
        if(d!=NULL)
        {
            d->notify(_id,on);
        }
    }
}

bool Device::isType(enum DeviceType type)
{
    return false;
}
