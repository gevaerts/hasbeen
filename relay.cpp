#include "Arduino.h"
#include "relay.h"
#include "relayboard.h"
#include "setup.h"

void Relay::init()
{
    setOn(_relayState);
}

Relay::Relay(int id, int board, int relay, enum DeviceType type) : Device(id, type)
{
    _board = board;
    _relay = relay;
    _relayState = 0;
    init();
}

Relay::Relay(int id, int board, int relay): Relay(id, relay, RELAY)
{
}

Relay::Relay(int id, unsigned char *initData): Device(id,initData)
{
    Serial.println(F("Restoring relay data"));
    _board = initData[_offset++];
    _relay = initData[_offset++];
    _relayState = 0; // TODO: nvram stuff
    init();
}

Relay::~Relay()
{
    Serial.println(F("Deleting relay"));
}

int Relay::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving relay data"));

    int offset = Device::saveConfig(initData);
    initData[offset++]=_board;
    initData[offset++]=_relay;
    return offset;
}

void Relay::printInfo()
{
    Device::printInfo();
    Serial.print(F("\trelay # "));
    Serial.println(_relay);
    Serial.print(F("\tcurrent relay state "));
    Serial.println(_relayState);
}

void Relay::setOn(int state)
{
    _relayState = state;
    Device *rb = Device::getDeviceForId(_board);
    if(rb != NULL && rb->getType()==RELAYBOARD)
    {
        ((RelayBoard *)rb)->setOn(_relay, state);
    }
    digitalWrite(relays[_relay],_relayState);
}

int Relay::relayState()
{
    return _relayState;
}
