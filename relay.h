#ifndef RELAY_H
#define RELAY_H

#include "device.h"

class Relay: public Device
{
    public:
        Relay(int id, int relay);
        Relay(int id, int relay, enum DeviceType type);
        Relay(int id, unsigned char *initdata);
        virtual ~Relay();
        virtual void press(int button,int previousState) {};
        virtual int respondsToButton(int button) {};
        virtual void printInfo();
    protected:
        virtual int saveConfig(unsigned char *initData);
        void setOn(int state);
        int relayState();
    private:
        void init();
        int _relay;
        int _relayState;
};
#endif
