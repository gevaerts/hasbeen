#include "Arduino.h"
#include "relay.h"
#include "relayboard.h"
#include "setup.h"
#include "nvram.h"

void Relay::saveState(uint8_t data)
{
    Serial.print(F("Saving relay state : 0x"));
    data<<=1;
    data |= (_relayState & 0x01);
    Serial.println(data, HEX);
    Device::saveState(data);
}

uint8_t Relay::restoreState()
{
    uint8_t data = Device::restoreState();
    _relayState = data & 0x01;
    data >>=1;
    Serial.print(F("Restoring relay state to "));
    Serial.println(_relayState);
    setOn(_relayState);
    return data;
}

Relay::Relay(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay, enum DeviceType type) : Device(id, nvSlot, type)
{
    _board = board;
    _relay = relay;
    _relayState = 0;
    setOn(_relayState);
}

Relay::Relay(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay): Relay(id, nvSlot, board, relay, RELAY)
{
}

Relay::Relay(uint8_t id, unsigned char *initData): Device(id,initData)
{
    Serial.println(F("Restoring relay data"));
    _board = initData[_offset++];
    _relay = initData[_offset++];
    _invert = initData[_offset++];
    // _relayState will be restored in restoreState(), so nothing to do here
}

Relay::~Relay()
{
    Serial.println(F("Deleting relay"));
}

uint8_t Relay::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving relay data"));

    uint8_t offset = Device::saveConfig(initData);
    initData[offset++]=_board;
    initData[offset++]=_relay;
    initData[offset++]=_invert;
    return offset;
}

void Relay::printInfo()
{
    Device::printInfo();
    Serial.print(F("\tboard # "));
    Serial.println(_board);
    Serial.print(F("\trelay # "));
    Serial.println(_relay);
    Serial.print(F("\tinvert? "));
    Serial.println(_invert);
    Serial.print(F("\tcurrent relay state "));
    Serial.println(_relayState);
}

void Relay::setOn(uint8_t state)
{
    _relayState = state;
    Device *rb = Device::getDeviceForId(_board);
    if(rb != NULL && rb->getType()==RELAYBOARD)
    {
        if(_invert) state = !state;
        ((RelayBoard *)rb)->setOn(_relay, state);
    }
}

uint8_t Relay::relayState()
{
    return _relayState;
}

void Relay::printDefinition(uint8_t first)
{
    {
        char buffer[40];
        sprintf(buffer,"setinvert %d %d",getId(), _invert);
        Serial.println(buffer);
    }
    Device::printDefinition(0);
};

bool Relay::isType(enum DeviceType type)
{
    if(type == RELAY)
        return true;
    else
        return Device::isType(type);
}
