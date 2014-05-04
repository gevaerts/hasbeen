#ifndef RELAY_H
#define RELAY_H

#include "device.h"

class Relay: public Device
{
    public:
        Relay(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay);
        Relay(uint8_t id, uint8_t nvSlot, uint8_t board, uint8_t relay, enum DeviceType type);
        Relay(uint8_t id, unsigned char *initdata);
        virtual ~Relay();
        virtual void press(uint8_t button,uint8_t previousState) {};
        virtual void printInfo();
        virtual void on() {setOn(1); Device::on();}
        virtual void off() {setOn(0); Device::off();}
        virtual char *getTypeName() {return "Relay";};
        virtual void printDefinition(uint8_t first);
        virtual bool isType(enum DeviceType type);
        virtual void setInvert(uint8_t invert) {_invert = invert;};
        virtual void loop() {};
    protected:
        virtual uint8_t saveConfig(unsigned char *initData);
        virtual uint8_t restoreState();
        virtual void saveState(uint8_t data);
        void setOn(uint8_t state);
        uint8_t relayState();
        uint8_t getRelay() {return _relay;}
        uint8_t getBoard() {return _board;}
    private:
        uint8_t _relay;
        uint8_t _board;
        uint8_t _relayState;
        uint8_t _invert;
};
#endif
