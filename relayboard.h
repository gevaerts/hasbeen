#ifndef RELAYBOARD_H
#define RELAYBOARD_H

#include "device.h"
#include <bv4627.h>
#include <Wire.h>

class RelayBoard: public Device
{
    public:
        RelayBoard(int id, int address);
        RelayBoard(int id, unsigned char *initdata);

        virtual ~RelayBoard();
        virtual void printInfo();
        void setOn(int relay, int state);
    protected:
        virtual int saveConfig(unsigned char *initData);
    private:
        void init();
        int _address;
        BV4627 *_board;
};
#endif
