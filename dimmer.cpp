#include "Arduino.h"
#include "dimmer.h"
#include "setup.h"

Dimmer::Dimmer(int relay,int buttonPlus,int buttonMin,int pwm):Relay(relay)
{
    _buttonPlus = buttonPlus;
    _buttonMin = buttonMin;
    _pwm = pwm;
    _brightness = DIMMER_STEPS / 2;

    _min = 8;
    _max = 128;

    pinMode(pwms[pwm],OUTPUT);
    writeBrightness();
    setOn(1);
}

void Dimmer::writeBrightness()
{
    // TODO calibration
    // TODO non-linear scaling?
    int absoluteBrightness = _min + (_brightness * (_max - _min) / DIMMER_STEPS);
    analogWrite(pwms[_pwm],~absoluteBrightness);
    //Serial.print("current brightness ");
    //Serial.println(absoluteBrightness, DEC);
}

int Dimmer::respondsToButton(int button)
{
    return (button == _buttonPlus || button == _buttonMin);
}

void Dimmer::press(int button,int previousState)
{
    Serial.print("Dimmer press ");
    // Dimmers will handle continuous presses, so we can ignore previousState
    int switchOff = 0;
    if(button == _buttonPlus)
    {
        Serial.print("Increment ");
        Serial.println(_pwm,DEC);

        int otherState = digitalRead(buttons[_buttonMin]);
        if(otherState == 0)
            switchOff = 1;
        else
            _brightness = constrain(_brightness + 1, 0, DIMMER_STEPS);
    }
    else if(button == _buttonMin)
    {
        Serial.print("Decrement ");
        Serial.println(_pwm,DEC);

        int otherState = digitalRead(buttons[_buttonPlus]);
        if(otherState == 0)
            switchOff = 1;
        else
            _brightness = constrain(_brightness - 1, 0, DIMMER_STEPS);
    }
    Serial.print("Dimmer brightness ");
        Serial.println(_brightness,DEC);
    if(_brightness == 0)
        switchOff = 1;

    if(switchOff)
    {
        setOn(0);
    }
    else
    {
        setOn(1);
    }
    writeBrightness();
}
