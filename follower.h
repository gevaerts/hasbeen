#ifndef FOLLOWER_H
#define FOLLOWER_H

#include "relay.h"

class Follower: public Relay
{
    public:
        Follower(uint8_t id, uint8_t board, uint8_t relay,uint8_t master, uint16_t delayOn, uint16_t delayOff);
        Follower(uint8_t id, unsigned char *initData);
        virtual ~Follower();
        virtual void printInfo();
        virtual void notify(uint8_t device, bool on);
        virtual char *getTypeName() {return "Follower";};
        virtual void printDefinition(uint8_t first);
        virtual bool isType(enum DeviceType type);
        virtual void loop();
    protected:
        virtual uint8_t saveConfig(unsigned char *initData);
    private:
        void init();
        uint8_t _master;
        uint16_t _delayOn;
        uint16_t _delayOff;
        long _onRequest;
        long _offRequest;
        uint8_t _waitingForOn;
        uint8_t _waitingForOff;
};
#endif
