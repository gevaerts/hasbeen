#ifndef DELAYEDGROUP_H
#define DELAYEDGROUP_H

#include "device.h"

class DelayedGroup: public Device
{
    public:
        DelayedGroup(uint8_t id, uint8_t master, uint16_t delayOn, uint16_t delayOff);
        DelayedGroup(uint8_t id, unsigned char *initData);
        virtual ~DelayedGroup();
        virtual void printInfo();
        void addMember(uint8_t id);
        void removeMember(uint8_t id);
        bool isMember(uint8_t id);
        virtual void on();
        virtual void off();
        virtual void notify(uint8_t device, bool on);
        virtual char *getTypeName() {return "DelayedGroup";};
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
        uint8_t _memberBitmap[NUM_DEVICES / 8];

        unsigned long _onRequest;
        unsigned long _offRequest;
        uint8_t _waitingForOn;
        uint8_t _waitingForOff;
};
#endif
