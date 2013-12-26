#ifndef DIMMER_H
#define DIMMER_H

#define DIMMER_STEPS 16

#include "relay.h"

class Dimmer: public Relay
{
    public:
        Dimmer(int relay,int buttonPlus,int buttonMin,int pwm);
        virtual void press(int button,int previousState);
        virtual int respondsToButton(int button);
    private:
        void writeBrightness();
        int _buttonPlus;
        int _buttonMin;
        int _pwm;
        int _brightness;
        int _min;
        int _max;
};
#endif
