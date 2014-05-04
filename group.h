#ifndef GROUP_H
#define GROUP_H

#include "device.h"

class Group: public Device
{
    public:
        Group(uint8_t id, unsigned char *initdata);
        Group(uint8_t id, uint8_t button);
        virtual ~Group();
        virtual void printInfo();
        void addMember(uint8_t id);
        void removeMember(uint8_t id);
        bool isMember(uint8_t id);
        virtual void on();
        virtual void off();
        virtual void press(uint8_t button,uint8_t previousState);
        virtual char *getTypeName() {return "Group";};
        virtual void printDefinition(uint8_t first);
        virtual bool isType(enum DeviceType type);
    protected:
        virtual uint8_t saveConfig(unsigned char *initData);
    private:
        uint8_t _memberBitmap[NUM_DEVICES / 8];
        uint8_t _button;
        void init();
};
#endif
