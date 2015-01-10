#include "DS1307RTC.h"

#include <Time.h>

#include <Arduino.h>
#include <EEPROM.h>
#include <I2C.h>
#include "setup.h"
#include "lightpoint.h"
#include "dimmer.h"
#include "relayboard.h"
#include "group.h"
#include "follower.h"
#include "pulser.h"
#include "delayedgroup.h"
#include "device.h"
#include <avr/wdt.h>

#define STATUS1 14
#define STATUS2 15

uint8_t buttonState[NUM_BUTTONS];
uint8_t buttonCount[NUM_BUTTONS];
unsigned long iteration;
unsigned long lastChange[NUM_BUTTONS];

uint8_t buttonAliases[NUM_BUTTONS];

long longest = -1;
uint8_t verbose = 0;

int freeRam ()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup()
{
    wdt_disable();

    pinMode(STATUS1,OUTPUT);
    pinMode(STATUS2,OUTPUT);

    status1(1);
    status2(1);
    I2c.begin();
    I2c.timeOut(1000);
    Serial1.begin(19200);
    Serial1.println(F("Init begin"));

    setSyncProvider(RTC.get);
    if(timeStatus()!= timeSet)
    {
        Serial1.println(F("Unable to sync with the RTC"));
    }
    else
    {
        Serial1.println(F("RTC has set the system time"));
        digitalClockDisplay();
    }

    for(uint8_t i=0;i<NUM_BUTTONS;i++)
    {
        buttonAliases[i]=EEPROM.read(ALIAS_EEPROM_BASE + i);
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

    Serial1.println(F("Init done"));
    Serial1.print(F("Free memory: "));
    Serial1.println(freeRam());
    status1(0);
    status2(0);
}

void digitalClockDisplay(){
    // digital clock display of the time
    Serial1.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial1.print(" ");
    Serial1.print(day());
    Serial1.print(" ");
    Serial1.print(month());
    Serial1.print(" ");
    Serial1.print(year());
    Serial1.println();
}

void printDigits(int digits){
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial1.print(":");
    if(digits < 10)
        Serial1.print('0');
    Serial1.print(digits);
}


char line[64];
char lidx = 0;
bool echo = false;

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
            Serial1.println(F("Parser error"));
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
    else if(!strcmp(tokens[0],"stats"))
    {
        Serial1.print(F("Milliseconds since start "));
        Serial1.println(millis());
        Serial1.print(F("Loops since start "));
        Serial1.println(iteration);
        Serial1.print(F("Average loop duration "));
        Serial1.println(millis()/iteration);
        Serial1.print(F("Longest loop duration "));
        Serial1.println(longest);
    }
    else if(!strcmp(tokens[0],"scan"))
    {
        i2cScan();
    }
    else if(!strcmp(tokens[0],"echo"))
    {
        if (tidx == 2 && !strcmp(tokens[1],"off"))
        {
            echo = false;
        }
        else
        {
            echo = true;
        }
    }
    else if(!strcmp(tokens[0],"cleareeprom") && tidx == 1)
    {
        for(uint8_t i=0;i<NUM_BUTTONS;i++)
        {
            EEPROM.write(ALIAS_EEPROM_BASE + i, -1);
        }
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
            for(uint8_t i=0;i<NUM_BUTTONS;i++)
            {
                if(buttonAliases[i] != 0xFF)
                {
                    Serial1.print(F("alias "));
                    Serial1.print(i,DEC);
                    Serial1.print(F(" "));
                    Serial1.print(buttonAliases[i],DEC);
                    Serial1.println();
                }
            }
        }
        Serial1.println(F("save"));
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
            Serial1.println(F("on <deviceId>"));
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
            Serial1.println(F("off <deviceId>"));
        }
    }
    else if(!strcmp(tokens[0],"devices"))
    {
        for(uint8_t i=0;i<NUM_DEVICES;i++)
        {
            Device *d = Device::getDeviceForId(i);
            if(d!=NULL)
            {
                Serial1.print(i);
                Serial1.print(F(" : "));
                Serial1.println(d->getName());
            }
        }
    }
    else if(!strcmp(tokens[0],"buttons"))
    {
        for(uint8_t i=0;i<NUM_BUTTONS;i++)
        {
            Serial1.print(i);
            Serial1.print(F(" : "));
            Serial1.print(buttonState[i]);
            Device *d = Device::getDeviceForButton(i);
            if(d!=NULL)
            {
                Serial1.print(F(" : "));
                Serial1.print(d->getId());
                Serial1.print(F(" : "));
                Serial1.println(d->getName());
            }
            else
            {
                Serial1.println(F(" : <>"));
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
    else if(!strcmp(tokens[0],"calibrate") && tidx == 4)
    {
        uint8_t id = atoi(tokens[1]);
        uint8_t min = atoi(tokens[2]);
        uint8_t max = atoi(tokens[3]);
        Device *d = Device::getDeviceForId(id);
        if(d != NULL && d->getType() == DIMMEDLIGHT)
        {
            Dimmer *dim = (Dimmer *)d;
            dim->setMin(min);
            dim->setMax(min);
            d->printInfo();
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
            else if(d != NULL && d->getType() == DELAYEDGROUP)
            {
                ((DelayedGroup *)d)->addMember(id);
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
            else if(d != NULL && d->getType() == DELAYEDGROUP)
            {
                ((DelayedGroup *)d)->removeMember(id);
                d->printInfo();
            }
        }
        else
        {
            Serial1.println(F("group {add|remove} <groupId> <deviceId>"));
        }
    }
    else if(!strcmp(tokens[0],"define") && tidx > 1)
    {
        if(!strcmp(tokens[1],"dimmer"))
        {
            if(tidx != 9)
            {
                Serial1.println(F("define dimmer <id> <nvSlot> <board> <relay> <buttonPlus> <buttonMin> <pwm>"));
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
                    Serial1.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial1.println(F("id not empty"));
                else if(relay < 1 || relay > 8)
                    Serial1.println(F("relay out of range"));
                else if(buttonPlus < 0 || buttonPlus >= NUM_BUTTONS)
                    Serial1.println(F("buttonPlus out of range"));
                else if(Device::getDeviceForButton(buttonPlus) != NULL)
                    Serial1.println(F("buttonPlus already in use"));
                else if(buttonMin < 0 || buttonMin >= NUM_BUTTONS)
                    Serial1.println(F("buttonMin out of range"));
                else if(Device::getDeviceForButton(buttonMin) != NULL)
                    Serial1.println(F("buttonMin already in use"));
                else if(pwm < 0 || pwm >= NUM_PWMS)
                    Serial1.println(F("pwm out of range"));
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
                Serial1.println(F("define group <id> <button>"));
            }
            else
            {
                uint8_t id, button;
                id = atoi(tokens[2]);
                button = atoi(tokens[3]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial1.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial1.println(F("id not empty"));
                else if(button < 0 || button >= NUM_BUTTONS)
                    Serial1.println(F("button out of range"));
                else if(Device::getDeviceForButton(button) != NULL)
                    Serial1.println(F("button already in use"));
                else
                {
                    Device *d = new Group(id, button);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"delayedgroup"))
        {
            if(tidx != 6)
            {
                Serial1.println(F("define delayedgroup <id> <master> <delayOn> <delayOff>"));
            }
            else
            {
                uint8_t id, master;
                uint16_t delayOn, delayOff;
                id = atoi(tokens[2]);
                master = atoi(tokens[3]);
                delayOn = atoi(tokens[4]);
                delayOff = atoi(tokens[5]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial1.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial1.println(F("id not empty"));
                else if(master < 0 || master >= NUM_DEVICES)
                    Serial1.println(F("master out of range"));
                else
                {
                    Device *d = new DelayedGroup(id, master, delayOn, delayOff);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"pulser"))
        {
            if(tidx != 7)
            {
                Serial1.println(F("define pulser <id> <board> <relay> <button> <time>"));
            }
            else
            {
                uint8_t id, board, relay, button;
                uint16_t time;
                id = atoi(tokens[2]);
                board = atoi(tokens[3]);
                relay = atoi(tokens[4]);
                button = atoi(tokens[5]);
                time = atoi(tokens[6]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial1.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial1.println(F("id not empty"));
                else if(relay < 1 || relay > 8)
                    Serial1.println(F("relay out of range"));
                else if(button < 0 || button >= NUM_BUTTONS)
                    Serial1.println(F("button out of range"));
                else if(Device::getDeviceForButton(button) != NULL)
                    Serial1.println(F("button already in use"));
                else
                {
                    Device *d = new Pulser(id, board, relay, button, time);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"follower"))
        {
            if(tidx != 8)
            {
                Serial1.println(F("define follower <id> <board> <relay> <master> <delayOn> <delayOff"));
            }
            else
            {
                uint8_t id, board, relay, master;
                uint16_t delayOn, delayOff;
                id = atoi(tokens[2]);
                board = atoi(tokens[3]);
                relay = atoi(tokens[4]);
                master = atoi(tokens[5]);
                delayOn = atoi(tokens[6]);
                delayOff = atoi(tokens[7]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial1.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial1.println(F("id not empty"));
                else if(relay < 1 || relay > 8)
                    Serial1.println(F("relay out of range"));
                else if(master < 0 || master >= NUM_DEVICES)
                    Serial1.println(F("master out of range"));
                else
                {
                    Device *d = new Follower(id, board, relay, master, delayOn, delayOff);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"lightpoint"))
        {
            if(tidx != 7)
            {
                Serial1.println(F("define lightpoint <id> <nvSlot> <board> <relay> <button>"));
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
                    Serial1.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial1.println(F("id not empty"));
                else if(relay < 1 || relay > 8)
                    Serial1.println(F("relay out of range"));
                else if(button < 0 || button >= NUM_BUTTONS)
                    Serial1.println(F("button out of range"));
                else if(Device::getDeviceForButton(button) != NULL)
                    Serial1.println(F("button already in use"));
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
                Serial1.println(F("define relayboard <id> <address>"));
            }
            else
            {
                uint8_t id, address;
                id = atoi(tokens[2]);
                address = atoi(tokens[3]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial1.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial1.println(F("id not empty"));
                else
                {
                    Device *d = new RelayBoard(id, address);
                    d->printInfo();
                }
            }
        }
        else
        {
            Serial1.println(F("define {dimmer|lightpoint|relayboard} ..."));
        }
    }
    else if(!strcmp(tokens[0],"alias"))
    {
        if(tidx == 3)
        {
            uint8_t button = atol(tokens[1]);
            uint8_t mapped_to = atol(tokens[2]);
            buttonAliases[button]=mapped_to;
        }
        else
        {
            Serial1.println(F("alias <button> <mapped_to>"));
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
                    Serial1.print(F("Saving "));
                    Serial1.print(i);
                    Serial1.print(F(" ..."));
                    d->saveSettings();
                    Serial1.println(F("done"));
                }
            }
            for(uint8_t i=0;i<NUM_BUTTONS;i++)
            {
                EEPROM.write(ALIAS_EEPROM_BASE + i, buttonAliases[i]);
            }
        }
        else
        {
            uint8_t id = atoi(tokens[1]);
            Device *d = Device::getDeviceForId(id);
            if(d != NULL)
            {
                Serial1.print(F("Saving "));
                Serial1.print(id);
                Serial1.print(F(" ... "));
                d->saveSettings();
                Serial1.println(F(" ... done"));
            }
            // Save one
        }
    }
    /*
       else if(!strcmp(tokens[0],"reset"))
       {
       Serial1.println(F("Resetting"));
       Serial1.flush();

       noInterrupts();
       wdt_enable (WDTO_8S);
       while(1);
       }
     */
    else if(!strcmp(tokens[0],"time"))
    {
        digitalClockDisplay();
    }
    else if(!strcmp(tokens[0],"freeze"))
    {
        while(1);
    }
    else if(!strcmp(tokens[0],"settime"))
    {
        if(tidx == 2)
        {
            time_t t = atol(tokens[1]);
            Serial1.println(t);
            RTC.set(t);
            digitalClockDisplay();
        }
        else
        {
            Serial1.println(F("settime <time_t>"));
        }
    }
    else if(!strcmp(tokens[0],"verbose"))
    {
        if (tidx == 2 && !strcmp(tokens[1],"off"))
        {
            verbose = 0;
        }
        else
        {
            verbose = 1;
        }
    }
    else
    {
        Serial1.print(F("Syntax error with token "));
        Serial1.println(tokens[0]);
    }
}

void serialEvent1()
{
    Serial1.write(19);
    while(Serial1.available())
    {
        int c = Serial1.read();
        if(c >= 0)
        {
            if(echo)
                Serial1.write(c);
            if(lidx >= 63 || c == '\n' || c == '\r')
            {
                if(echo)
                    Serial1.println();
                line[lidx] = 0;
                Serial1.print(F("Input line : '"));
                Serial1.print(line);
                Serial1.println(F("'"));
                handleInput();
                lidx = 0;
                continue;
            }
            line[lidx++] = c;
        }
    }
    Serial1.write(17);
}

void i2cScan()
{
    I2c.scan();
}

uint8_t buttonStatus(uint8_t button)
{
    for(uint8_t i=0;i<NUM_BUTTONS;i++)
    {
        if(buttonAliases[i] == button)
        {
            if(buttonState[i] == PRESSED)
            {
                return PRESSED;
            }
        }
    }
    return RELEASED;
}

void status1(bool on)
{
    digitalWrite(STATUS1,on);
}

void status2(bool on)
{
    digitalWrite(STATUS2,on);
}

unsigned long last = 0;
void loop()
{

    delay(10);
    iteration++;

    if(iteration%50 == 0)
    {
        status2(1);
    }

    long interval = millis() - last;
    last = millis();
    if(interval > longest)
        longest = interval;

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
                uint8_t button = i;
                if(buttonAliases[i] != 0xFF)
                {
                    button = buttonAliases[i];
                }
                else
                {
                    Serial1.print(F("Unaliased button "));
                    Serial1.print(i,DEC);
                    Serial1.println(F(" pressed. This is not expected to work with all devices!"));
                }

                if(verbose)
                {
                    Serial1.print(F("Button "));
                    Serial1.print(i,DEC);
                    Serial1.print(F(" aliased to "));
                    Serial1.print(button,DEC);
                    Serial1.println(F(" pressed"));
                }
                Device *d = Device::getDeviceForButton(button);
                if(d != NULL)
                {
                    if(verbose)
                    {
                        Serial1.print(F("Handled by device "));
                        Serial1.println(d->getName());
                    }
                    uint8_t previous = buttonState[i];
                    if(changed)
                        previous = !buttonState[i];

                    d->press(button,previous);
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

    if(iteration%50 == 5)
    {
        status2(0);
    }
}
