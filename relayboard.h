#ifndef RELAYBOARD_H
#define RELAYBOARD_H

#include "device.h"
#include "bv4627.h"

class RelayBoard: public Device
{
    public:
        RelayBoard(uint8_t id, uint8_t address);
        RelayBoard(uint8_t id, unsigned char *initdata);

        virtual ~RelayBoard();
        virtual void printInfo();
        void setOn(uint8_t relay, uint8_t state);
        void delayedSetOn(uint8_t relay, uint8_t state, int seconds);
        void setAddress(uint8_t address);
        virtual char *getTypeName() {return "RelayBoard";};
        virtual void printDefinition(uint8_t first);
        virtual bool isType(enum DeviceType type);
    protected:
        virtual uint8_t saveConfig(unsigned char *initData);
    private:
        void init();
        uint8_t _address;
        BV4627 *_board;
};
#endif
