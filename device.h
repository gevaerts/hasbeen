#ifndef DEVICE_H
#define DEVICE_H

#define DEVICE_EEPROM_SIZE 32
#define DEVICE_EEPROM_BASE 128
#define DEVICE_EEPROM_INDEX 0
#define NUM_DEVICES 32


#include "setup.h"

enum DeviceType
{
    DIMMEDLIGHT,
    ONOFFLIGHT,
    RELAY,
    TIMEDRELAY,
    RELAYBOARD,
    UNDEFINED = 255
};

class Device
{
    public:
        // id is used to derive things like eeprom and nvram addresses
        Device(int id, enum DeviceType type);
        Device(int id, unsigned char *initData);
        virtual ~Device();
        void setName(char *name);
        void saveSettings();
        void restoreSettings();
        virtual void printInfo();
        virtual void press(int button,int previousState) {};
        static void restore(int id);
        static Device *getDeviceForButton(int button);
        static Device *getDeviceForId(int id);
        enum DeviceType getType()
        {
            return _type;
        }
    protected:
        virtual int saveConfig(unsigned char *initData);
        static void registerButton(int button, Device *device);
        int _offset;
    private:
        char _name[16];
        int _id;
        enum DeviceType _type;
        static unsigned char scratch[DEVICE_EEPROM_SIZE];
        static Device *_devicesByButton[32]; //TODO: NUM_BUTTONS
        static Device *_devicesById[NUM_DEVICES]; 
};
#endif
