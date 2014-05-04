#ifndef DEVICE_H
#define DEVICE_H

#define DEVICE_EEPROM_SIZE 32
#define DEVICE_EEPROM_BASE 128
#define DEVICE_EEPROM_INDEX 0
#define NUM_DEVICES 64


#include "setup.h"

enum DeviceType
{
    DIMMEDLIGHT,
    ONOFFLIGHT,
    RELAY,
    TIMEDRELAY,
    RELAYBOARD,
    GROUP,
    FOLLOWER,
    PULSER,
    UNDEFINED = 255
};

#define NO_NVSLOT 0xff

class Device
{
    public:
        // id is used to derive things like eeprom and nvram addresses
        Device(uint8_t id, uint8_t nvSlot, enum DeviceType type);
        Device(uint8_t id, unsigned char *initData);
        virtual ~Device();
        void setName(char *name);
        char *getName() {return _name;}
        void saveSettings();
        virtual void printInfo();
        virtual void press(uint8_t button,uint8_t previousState) {};
        static void restore(uint8_t id);
        static Device *getDeviceForButton(uint8_t button);
        static Device *getDeviceForId(uint8_t id);
        virtual void on();
        virtual void off();
        virtual char *getTypeName() {return "Device";};
        virtual void printDefinition(uint8_t first);
        virtual void loop() {};
        enum DeviceType getType()
        {
            return _type;
        }
        virtual bool isType(enum DeviceType type);
        uint8_t getId() {return _id;}
    protected:
        virtual uint8_t saveConfig(unsigned char *initData);
        virtual uint8_t restoreState();
        virtual void saveState(uint8_t data);
        virtual void notify(uint8_t device, bool on) {};
        static void registerButton(uint8_t button, Device *device);
        static void unregisterButtons(Device *device);
        uint8_t getNVSlot() {return _nvSlot;}
        uint8_t _offset;
        void notifyAll(bool on);
    private:
        char _name[16];
        uint8_t _id;
        uint8_t _nvSlot;
        enum DeviceType _type;
        static unsigned char scratch[DEVICE_EEPROM_SIZE];
        static Device *_devicesByButton[NUM_BUTTONS];
        static Device *_devicesById[NUM_DEVICES];
};
#endif
