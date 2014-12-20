#include "Arduino.h"
#include "relay.h"
#include "relayboard.h"
#include "setup.h"
#include "nvram.h"

void Relay::saveState(uint8_t data)
{
    data<<=1;
    data |= (_relayState & 0x01);
    Device::saveState(data);
}

uint8_t Relay::restoreState()
{
    uint8_t data = Device::restoreState();
    _relayState = data & 0x01;
    data >>=1;
    setOn(_relayState);
    return data;
}

Relay::Relay(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay, enum DeviceType type) : Device(id, nvSlot, type)
{
    _board = board;
    _relay = relay;
    _relayState = 0;
    _invert = 0;
    _lastRefresh = 0;
    setOn(_relayState);
}

Relay::Relay(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay): Relay(id, nvSlot, board, relay, RELAY)
{
}

Relay::Relay(uint8_t id, unsigned char *initData): Device(id,initData)
{
    _board = initData[_offset++];
    _relay = initData[_offset++];
    _invert = initData[_offset++];
    _lastRefresh = 0;
    // _relayState will be restored in restoreState(), so nothing to do here
}

Relay::~Relay()
{
}

uint8_t Relay::saveConfig(unsigned char *initData)
{
    uint8_t offset = Device::saveConfig(initData);
    initData[offset++]=_board;
    initData[offset++]=_relay;
    initData[offset++]=_invert;
    return offset;
}

void Relay::printInfo()
{
    Device::printInfo();
    Serial1.print(F("\tboard # "));
    Serial1.println(_board);
    Serial1.print(F("\trelay # "));
    Serial1.println(_relay);
    Serial1.print(F("\tinvert? "));
    Serial1.println(_invert);
    Serial1.print(F("\tcurrent relay state "));
    Serial1.println(_relayState);
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
    notifyAll(_relayState);
}

uint8_t Relay::relayState()
{
    return _relayState;
}

void Relay::printDefinition(uint8_t first)
{
    if(_invert != 0)
    {
        char buffer[40];
        sprintf(buffer,"setinvert %d %d",getId(), _invert);
        Serial1.println(buffer);
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

void Relay::loop()
{
#ifdef USE_RELAY_FAILSAFE
    if(_invert) // inverted relays are the important ones!
    {
        // Every 10 seconds, we request a delayed relay off (which means on for inverted devices).
        // This will make the lights go on if we lose i2c or otherwise lock up.
        if (millis() > _lastRefresh + 10000)
        {
            if(verbose)
            {
                Serial1.print(F("delayed on for device "));
                Serial1.println(getId());
            }
            _lastRefresh = millis();
            Device *rb = Device::getDeviceForId(_board);
            if(rb != NULL && rb->getType()==RELAYBOARD)
            {
                ((RelayBoard *)rb)->delayedSetOn(_relay, 0, 30);
            }
        }
    }
#endif
    Device::loop();
}

