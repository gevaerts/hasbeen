#include <DS1307RTC.h>

#include <Time.h>

#include <Arduino.h>
#include <EEPROM.h>
#include <bv4627.h>
#include <Wire.h>
#include "setup.h"
#include "lightpoint.h"
#include "dimmer.h"
#include "relayboard.h"
#include "group.h"
#include "follower.h"
#include "device.h"
#include <avr/wdt.h>

#define STATUS1 22
#define STATUS2 23

uint8_t buttonState[NUM_BUTTONS]; // NUM_BUTTONS
uint8_t buttonCount[NUM_BUTTONS]; // NUM_BUTTONS
unsigned long iteration;
unsigned long lastChange[NUM_BUTTONS]; // NUM_BUTTONS

int freeRam ()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup()
{
    wdt_disable();
    Wire.begin();
    Serial.begin(115200);
    Serial.println(F("Init begin"));

    setSyncProvider(RTC.get);
    if(timeStatus()!= timeSet)
    {
        Serial.println(F("Unable to sync with the RTC"));
    }
    else
    {
        Serial.println(F("RTC has set the system time"));
        digitalClockDisplay();
    }

    for(uint8_t i=0;i<NUM_DEVICES;i++)
    {
        Device::restore(i);
    }
    for(uint8_t i=0;i<ARRAY_SIZE(buttonState);i++)
    {
        pinMode(buttons[i],INPUT);
        digitalWrite(buttons[i],HIGH); // Enable pull-up

        buttonState[i] = RELEASED;
    }

    Serial.println(F("Init done"));
    Serial.print(F("Free memory: "));
    Serial.println(freeRam());
}

void digitalClockDisplay(){
    // digital clock display of the time
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(" ");
    Serial.print(day());
    Serial.print(" ");
    Serial.print(month());
    Serial.print(" ");
    Serial.print(year());
    Serial.println();
}

void printDigits(int digits){
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(":");
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);
}


char line[64];
char lidx = 0;

void handleInput()
{
    char *tokens[10];
    char *t;
    uint8_t tidx = 0;
    t=strtok(line," ");

    while(t != NULL)
    {
        tokens[tidx++] = t;
        t = strtok(NULL, " ");
        if(tidx >= 10)
        {
            Serial.println(F("Parser error"));
            return;
        }
    }

    if(tidx == 0)
    {
        // No command was given
        return;
    }
    else if(!strcmp(tokens[0],"#"))
    {
        // comment
    }
    else if(!strcmp(tokens[0],"scan"))
    {
        i2cScan();
    }
    else if(!strcmp(tokens[0],"cleareeprom") && tidx == 1)
    {
        for(uint8_t i = 0; i < NUM_DEVICES; i++)
        {
            uint16_t baseAddress = i * DEVICE_EEPROM_SIZE + DEVICE_EEPROM_BASE;
            EEPROM.write(baseAddress, -1);
        }
    }
    else if(!strcmp(tokens[0],"showdefines"))
    {
        if(tidx == 2)
        {
            uint8_t i = atoi(tokens[1]);
            Device *d = Device::getDeviceForId(i);
            if(d!=NULL)
            {
                d->printDefinition(1);
            }
        }
        else
        {
            for(uint8_t i=0;i<NUM_DEVICES;i++)
            {
                Device *d = Device::getDeviceForId(i);
                if(d!=NULL)
                {
                    d->printDefinition(1);
                }
            }
        }
        Serial.println(F("save"));
    }
    else if(!strcmp(tokens[0],"on"))
    {
        if(tidx == 2)
        {
            Device *d = Device::getDeviceForId(atoi(tokens[1]));
            if(d!=NULL)
                d->on();
        }
        else
        {
            Serial.println(F("on <deviceId>"));
        }
    }
    else if(!strcmp(tokens[0],"off"))
    {
        if(tidx == 2)
        {
            Device *d = Device::getDeviceForId(atoi(tokens[1]));
            if(d!=NULL)
                d->off();
        }
        else
        {
            Serial.println(F("off <deviceId>"));
        }
    }
    else if(!strcmp(tokens[0],"devices"))
    {
        for(uint8_t i=0;i<NUM_DEVICES;i++)
        {
            Device *d = Device::getDeviceForId(i);
            if(d!=NULL)
            {
                Serial.print(i);
                Serial.print(F(" : "));
                Serial.println(d->getName());
            }
        }
    }
    else if(!strcmp(tokens[0],"buttons"))
    {
        for(uint8_t i=0;i<NUM_BUTTONS;i++)
        {
            Serial.print(i);
            Serial.print(F(" : "));
            Serial.print(buttonState[i]);
            Device *d = Device::getDeviceForButton(i);
            if(d!=NULL)
            {
                Serial.print(F(" : "));
                Serial.print(d->getId());
                Serial.print(F(" : "));
                Serial.println(d->getName());
            }
            else
            {
                Serial.println(F(" : <>"));
            }
        }
    }
    else if(!strcmp(tokens[0],"show"))
    {
        if(tidx == 2)
        {
            Device *d = Device::getDeviceForId(atoi(tokens[1]));
            if(d!=NULL)
                d->printInfo();
        }
        else
        {
            for(uint8_t i=0;i<NUM_DEVICES;i++)
            {
                Device *d = Device::getDeviceForId(i);
                if(d!=NULL)
                    d->printInfo();
            }
        }
    }
    else if(!strcmp(tokens[0],"setname") && tidx > 2)
    {
        uint8_t id = atoi(tokens[1]);
        char name[16]="";
        for(uint8_t i=2;i<tidx;i++)
        {
            strncat(name,tokens[i],16);
            strncat(name," ",16);
        }
        Device *d = Device::getDeviceForId(id);
        if(d != NULL)
            d->setName(name);
    }
    else if(!strcmp(tokens[0],"setinvert") && tidx == 3)
    {
        uint8_t id = atoi(tokens[1]);
        uint8_t invert = atoi(tokens[2]);
        Device *d = Device::getDeviceForId(id);
        if(d != NULL && d->isType(RELAY))
        {
            ((Relay *)d)->setInvert(invert);
        }
    }
    else if(!strcmp(tokens[0],"setaddress") && tidx == 3)
    {
        uint8_t id = atoi(tokens[1]);
        uint8_t address = atoi(tokens[2]);
        Device *d = Device::getDeviceForId(id);
        if(d != NULL && d->getType() == RELAYBOARD)
        {
            ((RelayBoard *)d)->setAddress(address);
        }
    }
    else if(!strcmp(tokens[0],"delete") && tidx == 2)
    {
        uint8_t id = atoi(tokens[1]);
        Device *d = Device::getDeviceForId(id);
        if(d != NULL)
            delete d;
    }
    else if(!strcmp(tokens[0],"group"))
    {
        if (tidx == 4 && !strcmp(tokens[1],"add"))
        {
            uint8_t gid = atoi(tokens[2]);
            uint8_t id = atoi(tokens[3]);
            Device *d = Device::getDeviceForId(gid);
            if(d != NULL && d->getType() == GROUP)
            {
                ((Group *)d)->addMember(id);
                d->printInfo();
            }
        }
        else if (tidx == 4 && !strcmp(tokens[1],"remove"))
        {
            uint8_t gid = atoi(tokens[2]);
            uint8_t id = atoi(tokens[3]);
            Device *d = Device::getDeviceForId(gid);
            if(d != NULL && d->getType() == GROUP)
            {
                ((Group *)d)->removeMember(id);
                d->printInfo();
            }
        }
        else
        {
            Serial.println(F("group {add|remove} <groupId> <deviceId>"));
        }
    }
    else if(!strcmp(tokens[0],"define") && tidx > 1)
    {
        if(!strcmp(tokens[1],"dimmer"))
        {
            if(tidx != 9)
            {
                Serial.println(F("define dimmer <id> <nvSlot> <board> <relay> <buttonPlus> <buttonMin> <pwm>"));
            }
            else
            {
                uint8_t id, nvslot, board, relay, buttonPlus, buttonMin, pwm;
                id = atoi(tokens[2]);
                nvslot = atoi(tokens[3]);
                board = atoi(tokens[4]);
                relay = atoi(tokens[5]);
                buttonPlus = atoi(tokens[6]);
                buttonMin = atoi(tokens[7]);
                pwm = atoi(tokens[8]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial.println(F("id not empty"));
                else if(relay < 1 || relay > 8)
                    Serial.println(F("relay out of range"));
                else if(buttonPlus < 0 || buttonPlus >= NUM_BUTTONS)
                    Serial.println(F("buttonPlus out of range"));
                else if(Device::getDeviceForButton(buttonPlus) != NULL)
                    Serial.println(F("buttonPlus already in use"));
                else if(buttonMin < 0 || buttonMin >= NUM_BUTTONS)
                    Serial.println(F("buttonMin out of range"));
                else if(Device::getDeviceForButton(buttonMin) != NULL)
                    Serial.println(F("buttonMin already in use"));
                else
                {
                    Device *d = new Dimmer(id, nvslot, board, relay,buttonPlus,buttonMin,pwm);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"group"))
        {
            if(tidx != 4)
            {
                Serial.println(F("define group <id> <button>"));
            }
            else
            {
                uint8_t id, button;
                id = atoi(tokens[2]);
                button = atoi(tokens[3]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial.println(F("id not empty"));
                else if(button < 0 || button >= NUM_BUTTONS)
                    Serial.println(F("button out of range"));
                else if(Device::getDeviceForButton(button) != NULL)
                    Serial.println(F("button already in use"));
                else
                {
                    Device *d = new Group(id, button);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"follower"))
        {
            if(tidx != 9)
            {
                Serial.println(F("define lightpoint <id> <nvSlot> <board> <relay> <master> <delayOn> <delayOff"));
            }
            else
            {
                uint8_t id, nvslot, board, relay, master;
                uint16_t delayOn, delayOff;
                id = atoi(tokens[2]);
                nvslot = atoi(tokens[3]);
                board = atoi(tokens[4]);
                relay = atoi(tokens[5]);
                master = atoi(tokens[6]);
                delayOn = atoi(tokens[7]);
                delayOff = atoi(tokens[8]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial.println(F("id not empty"));
                else if(relay < 1 || relay > 8)
                    Serial.println(F("relay out of range"));
                else if(master < 0 || master >= NUM_DEVICES)
                    Serial.println(F("master out of range"));
                else
                {
                    Device *d = new Follower(id, nvslot, board, relay, master, delayOn, delayOff);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"lightpoint"))
        {
            if(tidx != 7)
            {
                Serial.println(F("define lightpoint <id> <nvSlot> <board> <relay> <button>"));
            }
            else
            {
                uint8_t id, nvslot, board, relay, button;
                id = atoi(tokens[2]);
                nvslot = atoi(tokens[3]);
                board = atoi(tokens[4]);
                relay = atoi(tokens[5]);
                button = atoi(tokens[6]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial.println(F("id not empty"));
                else if(relay < 1 || relay > 8)
                    Serial.println(F("relay out of range"));
                else if(button < 0 || button >= NUM_BUTTONS)
                    Serial.println(F("button out of range"));
                else if(Device::getDeviceForButton(button) != NULL)
                    Serial.println(F("button already in use"));
                else
                {
                    Device *d = new Lightpoint(id, nvslot, board, relay,button);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"relayboard"))
        {
            if(tidx != 4)
            {
                Serial.println(F("define relayboard <id> <address>"));
            }
            else
            {
                uint8_t id, address;
                id = atoi(tokens[2]);
                address = atoi(tokens[3]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial.println(F("id not empty"));
                else
                {
                    Device *d = new RelayBoard(id, address);
                    d->printInfo();
                }
            }
        }
        else
        {
            Serial.println(F("define {dimmer|lightpoint|relayboard} ..."));
        }
    }
    else if(!strcmp(tokens[0],"save"))
    {
        if(tidx == 1)
        {
            // Save all
            for(uint8_t i=0;i<NUM_DEVICES;i++)
            {
                Device *d = Device::getDeviceForId(i);
                if(d != NULL)
                {
                    Serial.print(F("Saving "));
                    Serial.print(i);
                    Serial.print(F(" ..."));
                    d->saveSettings();
                    Serial.println(F("done"));
                }
            }
        }
        else
        {
            uint8_t id = atoi(tokens[1]);
            Device *d = Device::getDeviceForId(id);
            if(d != NULL)
                d->saveSettings();
            // Save one
        }
    }
    /*
       else if(!strcmp(tokens[0],"reset"))
       {
       Serial.println(F("Resetting"));
       Serial.flush();

       noInterrupts();
       wdt_enable (WDTO_8S);
       while(1);
       }
     */
    else if(!strcmp(tokens[0],"time"))
    {
        digitalClockDisplay();
    }
    else
    {
        Serial.print(F("Syntax error with token "));
        Serial.println(tokens[0]);
    }
}

void serialEvent()
{
    while(Serial.available())
    {
        int c = Serial.read();
        if(c >= 0)
        {
            if(lidx >= 63 || c == '\n')
            {
                line[lidx] = 0;
                Serial.print(F("Input line : '"));
                Serial.print(line);
                Serial.println(F("'"));
                handleInput();
                lidx = 0;
                continue;
            }
            line[lidx++] = c;
        }
    }
}

void i2cScan()
{
    byte error, address;
    int nDevices;

    Serial.println(F("Scanning..."));

    nDevices = 0;
    for(address = 1; address < 127; address++ )
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print(F("I2C device found at address 0x"));
            if (address<16)
                Serial.print("0");
            Serial.println(address,HEX);

            nDevices++;
        }
        else if (error==4)
        {
            Serial.print(F("Unknow error at address 0x"));
            if (address<16)
                Serial.print("0");
            Serial.println(address,HEX);
        }
    }
    if (nDevices == 0)
        Serial.println(F("No I2C devices found\n"));
    else
        Serial.println(F("done\n"));

}

bool status = 0;
void loop()
{
    if(iteration%50 == 0)
    {
        status = !status;
        pinMode(STATUS1,OUTPUT);
        pinMode(STATUS2,OUTPUT);
        digitalWrite(STATUS1,status);
        digitalWrite(STATUS2,!status);
    }

    delay(10);
    iteration++;

    for(uint8_t i=0;i<NUM_BUTTONS;i++)
    {
        uint8_t value = digitalRead(buttons[i]);

        bool changed = 0;
        if(buttonState[i] != value)
        {
            buttonCount[i]++;
            if(buttonCount[i] > 5)
            {
                buttonState[i] = value;
                lastChange[i] = iteration;
                changed = 1;
            }
        }
        else
            buttonCount[i]=0;

        if((lastChange[i] - iteration) % 20 == 0) // Repeat every 20th cycle (~200ms, to verify)
        {
            if(buttonState[i] == PRESSED)
            {
                Serial.print(F("Button "));
                Serial.print(i,DEC);
                Serial.println((" pressed"));
                Device *d = Device::getDeviceForButton(i);
                if(d != NULL)
                {
                    uint8_t previous = buttonState[i];
                    if(changed)
                        previous = !buttonState[i];

                    d->press(i,previous);
                }
            }
        }
    }
    for(uint8_t i=0;i<NUM_DEVICES;i++)
    {
        Device *d = Device::getDeviceForId(i);
        if(d!=NULL)
        {
            d->loop();
        }
    }
}
