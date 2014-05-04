#ifndef PULSER_H
#define PULSER_H

#include "relay.h"

class Pulser: public Relay
{
    public:
        Pulser(uint8_t id, uint8_t board, uint8_t relay,uint8_t button, uint16_t time);
        Pulser(uint8_t id, unsigned char *initData);
        virtual ~Pulser();
        virtual void printInfo();
        virtual char *getTypeName() {return "Pulser";};
        virtual void printDefinition(uint8_t first);
        virtual bool isType(enum DeviceType type);
        virtual void press(uint8_t button,uint8_t previousState);
        virtual void loop();
    protected:
        virtual uint8_t saveConfig(unsigned char *initData);
    private:
        void init();
        uint8_t _button;
        uint16_t _time;
        uint8_t _waitingForOff;
        long _pressTime;
};
#endif
