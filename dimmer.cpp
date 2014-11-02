#include "Arduino.h"
#include "dimmer.h"
#include "setup.h"

void Dimmer::init()
{
    registerButton(_buttonPlus, this);
    registerButton(_buttonMin, this);

    pinMode(pwms[_pwm],OUTPUT);
}

Dimmer::Dimmer(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay, uint8_t buttonPlus,uint8_t buttonMin,uint8_t pwm):Relay(id, nvSlot, board, relay, DIMMEDLIGHT)
{
    _buttonPlus = buttonPlus;
    _buttonMin = buttonMin;
    _pwm = pwm;
    _brightness = DIMMER_STEPS / 2;
    _min = 8;
    _max = 128;
    init();
    writeBrightness();
    saveState(0);
}

Dimmer::Dimmer(uint8_t id, unsigned char *initData) : Relay(id, initData)
{
    _buttonPlus = initData[_offset++];
    _buttonMin = initData[_offset++];
    _pwm = initData[_offset++];
    // _brightness will be restored in restoreState(), so nothing to do here
    _min = initData[_offset++];
    _max = initData[_offset++];
    init();
}

Dimmer::~Dimmer()
{
}


void Dimmer::saveState(uint8_t data)
{
    data<<=4;
    data |= (_brightness & 0xf); // 16 steps
    Relay::saveState(data);
}

uint8_t Dimmer::restoreState()
{
    uint8_t data = Relay::restoreState();
    _brightness = data & 0x0f; // 16 steps
    data >>=4;
    writeBrightness();
    return data;
}



uint8_t Dimmer::saveConfig(unsigned char *initData)
{
    uint8_t offset = Relay::saveConfig(initData);
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
    // TODO autocalibration?
    // TODO non-linear scaling?
    uint8_t absoluteBrightness = _min + (_brightness * (_max - _min) / DIMMER_STEPS);
    if(_pwm >= 0 && _pwm < NUM_PWMS)
        analogWrite(pwms[_pwm],~absoluteBrightness);
}

void Dimmer::press(uint8_t button,uint8_t previousState)
{
    // Dimmers will handle continuous presses, so we can ignore previousState
    bool switchOff = 0;
    if(button == _buttonPlus)
    {
        uint8_t otherState = buttonStatus(buttons[_buttonMin]);
        if(otherState == 0)
            switchOff = 1;
        else
            _brightness = constrain(_brightness + 1, 0, DIMMER_STEPS);
    }
    else if(button == _buttonMin)
    {
        uint8_t otherState = buttonStatus(buttons[_buttonPlus]);
        if(otherState == 0)
            switchOff = 1;
        else
            _brightness = constrain(_brightness - 1, 0, DIMMER_STEPS);
    }
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
    saveState(0);
}

void Dimmer::printDefinition(uint8_t first)
{
    {
        char buffer[40];
        sprintf(buffer,"define dimmer %d %d %d %d %d %d %d",getId(), getNVSlot(), getBoard(), getRelay(), _buttonPlus, _buttonMin, _pwm);
        Serial.println(buffer);
        sprintf(buffer,"calibrate %d %d %d",getId(), _min, _max);
        Serial.println(buffer);
    }
    Relay::printDefinition(0);
};

bool Dimmer::isType(enum DeviceType type)
{
    if(type == DIMMEDLIGHT)
        return true;
    else
        return Relay::isType(type);
}
