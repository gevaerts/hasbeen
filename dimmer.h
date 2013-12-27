#ifndef DIMMER_H
#define DIMMER_H

#define DIMMER_STEPS 16

#include "relay.h"

class Dimmer: public Relay
{
    public:
        Dimmer(int id, int board, int relay,int buttonPlus,int buttonMin,int pwm);
        Dimmer(int id, unsigned char *initData);
        virtual ~Dimmer();
        virtual void press(int button,int previousState);
        virtual int respondsToButton(int button);
        virtual void printInfo();
    protected:
        virtual int saveConfig(unsigned char *initData);
    private:
        void init();
        void writeBrightness();
        int _buttonPlus;
        int _buttonMin;
        int _pwm;
        int _brightness;
        int _min;
        int _max;
};
#endif
