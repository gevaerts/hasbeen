#ifndef DIMMER_H
#define DIMMER_H

#define DIMMER_STEPS 15

#include "relay.h"

class Dimmer: public Relay
{
    public:
        Dimmer(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay,uint8_t buttonPlus,uint8_t buttonMin,uint8_t pwm);
        Dimmer(uint8_t id, unsigned char *initData);
        virtual ~Dimmer();
        virtual void press(uint8_t button,uint8_t previousState);
        virtual bool respondsToButton(uint8_t button);
        virtual void printInfo();
        virtual char *getTypeName() {return "Dimmer";};
        virtual void printDefinition(uint8_t first);
        virtual bool isType(enum DeviceType type);
    protected:
        virtual uint8_t saveConfig(unsigned char *initData);
        virtual uint8_t restoreState();
        virtual void saveState(uint8_t data);
    private:
        void init();
        void writeBrightness();
        uint8_t _buttonPlus;
        uint8_t _buttonMin;
        uint8_t _pwm;
        uint8_t _brightness;
        uint8_t _min;
        uint8_t _max;
};
#endif
