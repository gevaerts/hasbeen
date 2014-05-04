#include "Arduino.h"
#include "pulser.h"
#include "setup.h"

void Pulser::init()
{
    registerButton(_button, this);
    _waitingForOff = 0;
}

Pulser::Pulser(uint8_t id, uint8_t board, uint8_t relay, uint8_t button, uint16_t time):Relay(id, NO_NVSLOT, board, relay, PULSER)
{
    _button = button;
    _time = time;
    init();
}

Pulser::Pulser(uint8_t id, unsigned char *initData): Relay(id,initData)
{
    Serial.println(F("Restoring pulser data"));
    _button = initData[_offset++];
    _time = initData[_offset++]<<8;
    _time += initData[_offset++];
    init();
}

Pulser::~Pulser()
{
}

uint8_t Pulser::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving pulser data"));
    uint8_t offset = Relay::saveConfig(initData);
    initData[offset++]=_button;
    initData[offset++]=_time>>8;
    initData[offset++]=_time&0xff;
    return offset;
}

void Pulser::printInfo()
{
    Relay::printInfo();
    Serial.print(F("\tbutton # "));
    Serial.println(_button);
    Serial.print(F("\ttime "));
    Serial.println(_time);
}

void Pulser::printDefinition(uint8_t first)
{
    {
        char buffer[40];
        sprintf(buffer,"define pulser %d %d %d %d %d",getId(), getBoard(), getRelay(), _button, _time);
        Serial.println(buffer);
    }
    Device::printDefinition(0);
};

void Pulser::loop()
{
    if(_waitingForOff)
    {
        if((millis() - _pressTime)/1000 >= _time)
        {
            _waitingForOff = 0;
            setOn(0);
        }
    }
}

void Pulser::press(uint8_t button, uint8_t previousState)
{
    // If previousState was PRESSED, ignore this
    if((previousState == RELEASED) && button == _button)
    {
        setOn(1);
        _waitingForOff = 1;
        _pressTime = millis();
    }
}


bool Pulser::isType(enum DeviceType type)
{
    if(type == FOLLOWER)
        return true;
    else
        return Relay::isType(type);
}
