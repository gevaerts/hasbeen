#include "Arduino.h"
#include "dimmer.h"
#include "setup.h"

void Dimmer::init()
{
    registerButton(_buttonPlus, this);
    registerButton(_buttonMin, this);

    pinMode(pwms[_pwm],OUTPUT);
    writeBrightness();
    setOn(1);
}

Dimmer::Dimmer(int id, int relay,int buttonPlus,int buttonMin,int pwm):Relay(id, relay, DIMMEDLIGHT)
{
    _buttonPlus = buttonPlus;
    _buttonMin = buttonMin;
    _pwm = pwm;
    _brightness = DIMMER_STEPS / 2;

    _min = 8;
    _max = 128;
    init();
}

Dimmer::Dimmer(int id, unsigned char *initData) : Relay(id, initData)
{
    Serial.println(F("Restoring dimmer data"));
    _buttonPlus = initData[_offset++];
    _buttonMin = initData[_offset++];
    _pwm = initData[_offset++];
    _brightness = DIMMER_STEPS / 2; // TODO: nvram stuff
    _min = initData[_offset++];
    _max = initData[_offset++];
    init();
}

Dimmer::~Dimmer()
{
    Serial.println(F("Deleting dimmer"));
    registerButton(_buttonPlus, NULL);
    registerButton(_buttonMin, NULL);
}

int Dimmer::saveConfig(unsigned char *initData)
{
    Serial.println(F("Saving dimmer data"));
    int offset = Relay::saveConfig(initData);
    initData[offset++]=_buttonPlus;
    initData[offset++]=_buttonMin;
    initData[offset++]=_pwm;
    initData[offset++]=_min;
    initData[offset++]=_max;
    return offset;
}


void Dimmer::printInfo()
{
    Relay::printInfo();
    Serial.print(F("\tbutton + # "));
    Serial.println(_buttonPlus);
    Serial.print(F("\tbutton - # "));
    Serial.println(_buttonMin);
    Serial.print(F("\tpwm # "));
    Serial.println(_pwm);
    Serial.print(F("\tmin output "));
    Serial.println(_min);
    Serial.print(F("\tmax output "));
    Serial.println(_max);
    Serial.print(F("\tcurrent brightness "));
    Serial.println(_brightness);
}

void Dimmer::writeBrightness()
{
    // TODO calibration
    // TODO non-linear scaling?
    int absoluteBrightness = _min + (_brightness * (_max - _min) / DIMMER_STEPS);
    analogWrite(pwms[_pwm],~absoluteBrightness);
}

int Dimmer::respondsToButton(int button)
{
    return (button == _buttonPlus || button == _buttonMin);
}

void Dimmer::press(int button,int previousState)
{
    Serial.print(F("Dimmer press "));
    // Dimmers will handle continuous presses, so we can ignore previousState
    int switchOff = 0;
    if(button == _buttonPlus)
    {
        Serial.print(F("Increment "));
        Serial.println(_pwm,DEC);

        int otherState = digitalRead(buttons[_buttonMin]);
        if(otherState == 0)
            switchOff = 1;
        else
            _brightness = constrain(_brightness + 1, 0, DIMMER_STEPS);
    }
    else if(button == _buttonMin)
    {
        Serial.print(F("Decrement "));
        Serial.println(_pwm,DEC);

        int otherState = digitalRead(buttons[_buttonPlus]);
        if(otherState == 0)
            switchOff = 1;
        else
            _brightness = constrain(_brightness - 1, 0, DIMMER_STEPS);
    }
    Serial.print(F("Dimmer brightness "));
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
