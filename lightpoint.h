#ifndef LIGHTPOINT_H
#define LIGHTPOINT_H

#include "relay.h"

class Lightpoint: public Relay
{
    public:
        Lightpoint(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay,uint8_t button);
        Lightpoint(uint8_t id, unsigned char *initData);
        virtual ~Lightpoint();
        virtual void press(uint8_t button,uint8_t previousState);
        virtual bool respondsToButton(uint8_t button);
        virtual void printInfo();
        virtual char *getTypeName() {return "Lightpoint";};
    protected:
        virtual uint8_t saveConfig(unsigned char *initData);
    private:
        void init();
        uint8_t _button;
};
#endif
