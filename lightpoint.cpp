#include "Arduino.h"
#include "lightpoint.h"
#include "setup.h"

Lightpoint::Lightpoint(int relay, int button):Relay(relay)
{
    _button = button;
}

int Lightpoint::respondsToButton(int button)
{
    return (button == _button);
}

void Lightpoint::press(int button, int previousState)
{
    Serial.print("Simple ");
    Serial.print(_button,DEC);
    Serial.print(" previous state ");
    Serial.println(previousState,DEC);

    // If previousState was PRESSED, ignore this
    if((previousState == RELEASED) && button == _button)
    {
        setOn(!relayState());
    }
}
