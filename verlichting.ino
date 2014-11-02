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
#include "pulser.h"
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
    else if(!strcmp(tokens[0],"stats"))
    {
        Serial.print(F("Milliseconds since start "));
        Serial.println(millis());
        Serial.print(F("Loops since start "));
        Serial.println(iteration);
        Serial.print(F("Average loop duration "));
        Serial.println(millis()/iteration);
        Serial.print(F("Longest loop duration "));
        Serial.println(longest);
    }
    else if(!strcmp(tokens[0],"scan"))
    {
        i2cScan();
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
                    Serial.print(F("alias "));
                    Serial.print(i,DEC);
                    Serial.print(F(" "));
                    Serial.print(buttonAliases[i],DEC);
                    Serial.print(F("\n"));
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
                else if(pwm < 0 || pwm >= NUM_PWMS)
                    Serial.println(F("pwm out of range"));
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
        else if(!strcmp(tokens[1],"pulser"))
        {
            if(tidx != 7)
            {
                Serial.println(F("define pulser <id> <board> <relay> <button> <time>"));
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
                    Device *d = new Pulser(id, board, relay, button, time);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"follower"))
        {
            if(tidx != 8)
            {
                Serial.println(F("define follower <id> <board> <relay> <master> <delayOn> <delayOff"));
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
                    Serial.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial.println(F("id not empty"));
                else if(relay < 1 || relay > 8)
                    Serial.println(F("relay out of range"));
                else if(master < 0 || master >= NUM_DEVICES)
                    Serial.println(F("master out of range"));
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
            Serial.println(F("alias <button> <mapped_to>"));
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
                Serial.print(F("Saving "));
                Serial.print(id);
                Serial.print(F(" ... "));
                d->saveSettings();
                Serial.println(F(" ... done"));
            }
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
    else if(!strcmp(tokens[0],"settime"))
    {
        if(tidx == 2)
        {
            time_t t = atol(tokens[1]);
            Serial.println(t);
            RTC.set(t);
            digitalClockDisplay();
        }
        else
        {
            Serial.println(F("settime <time_t>"));
        }
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

uint8_t buttonStatus(int button)
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

bool status = 0;
unsigned long last = 0;
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
                    button = buttonAliases[i];

                Serial.print(F("Button "));
                Serial.print(i,DEC);
                Serial.print(F(" aliased to "));
                Serial.print(button,DEC);
                Serial.println(F(" pressed"));
                Device *d = Device::getDeviceForButton(button);
                if(d != NULL)
                {
                    Serial.print(F("Handled by device "));
                    Serial.println(d->getName());
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
}
