#include "Arduino.h"
#include "relay.h"
#include "setup.h"

Relay::Relay(int relay)
{
    pinMode(relays[relay],OUTPUT);
    digitalWrite(relays[relay],LOW);

    _relay = relay;
    _relayState = 0;
}

void Relay::setOn(int state)
{
    _relayState = state;
    digitalWrite(relays[_relay],_relayState);
}

int Relay::relayState()
{
    return _relayState;
}
