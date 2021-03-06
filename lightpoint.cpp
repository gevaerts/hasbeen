#include "Arduino.h"
#include "lightpoint.h"
#include "setup.h"

void Lightpoint::init()
{
    registerButton(_button, this);
}

Lightpoint::Lightpoint(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay, uint8_t button):Relay(id, nvSlot, board, relay, ONOFFLIGHT)
{
    _button = button;
    init();
    saveState(0);
}

Lightpoint::Lightpoint(uint8_t id, unsigned char *initData): Relay(id,initData)
{
    _button = initData[_offset++];
    init();
}

Lightpoint::~Lightpoint()
{
}

uint8_t Lightpoint::saveConfig(unsigned char *initData)
{
    uint8_t offset = Relay::saveConfig(initData);
    initData[offset++]=_button;
    return offset;
}

void Lightpoint::printInfo()
{
    Relay::printInfo();
    Serial.print(F("\tbutton # "));
    Serial.println(_button);
}

void Lightpoint::press(uint8_t button, uint8_t previousState)
{
    // If previousState was PRESSED, ignore this
    if((previousState == RELEASED) && button == _button)
    {
        setOn(!relayState());
    }
    saveState(0);
}

void Lightpoint::printDefinition(uint8_t first)
{
    {
        char buffer[40];
        sprintf(buffer,"define lightpoint %d %d %d %d %d",getId(), getNVSlot(), getBoard(), getRelay(), _button);
        Serial.println(buffer);
    }
    Relay::printDefinition(0);
};

bool Lightpoint::isType(enum DeviceType type)
{
    if(type == ONOFFLIGHT)
        return true;
    else
        return Relay::isType(type);
}
