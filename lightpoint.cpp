#include "Arduino.h"
#include "lightpoint.h"
#include "setup.h"

void Lightpoint::init()
{
    registerButton(_button, this);
}

Lightpoint::Lightpoint(int id, int board, int relay, int button):Relay(id, board, relay, ONOFFLIGHT)
{
    _button = button;
    init();
}

Lightpoint::Lightpoint(int id, unsigned char *initData): Relay(id,initData)
{
    Serial.println(F("Restoring lightpoint data"));
    _button = initData[_offset++];
    init();
}

Lightpoint::~Lightpoint()
{
    Serial.println(F("Deleting lightpoint"));
    registerButton(_button, NULL);
}

int Lightpoint::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving lightpoint data"));
    int offset = Relay::saveConfig(initData);
    initData[offset++]=_button;
    return offset;
}

void Lightpoint::printInfo()
{
    Relay::printInfo();
    Serial.print(F("\tbutton # "));
    Serial.println(_button);
}

int Lightpoint::respondsToButton(int button)
{
    return (button == _button);
}

void Lightpoint::press(int button, int previousState)
{
    Serial.print(F("Simple "));
    Serial.print(_button,DEC);
    Serial.print(F(" previous state "));
    Serial.println(previousState,DEC);

    // If previousState was PRESSED, ignore this
    if((previousState == RELEASED) && button == _button)
    {
        setOn(!relayState());
    }
}
