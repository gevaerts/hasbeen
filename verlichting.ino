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
#include "device.h"
#include <avr/wdt.h>

#define STATUS1 22
#define STATUS2 23

uint8_t buttonState[32]; // NUM_BUTTONS
uint8_t buttonCount[32]; // NUM_BUTTONS
unsigned long iteration;
unsigned long lastChange[32]; // NUM_BUTTONS

void setup()
{
    Wire.begin();
    Serial.begin(9600);
    Serial.println(F("Init begin"));
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus()!= timeSet)
    {
        Serial.println("Unable to sync with the RTC");
    }
    else
    {
        Serial.println("RTC has set the system time");
        digitalClockDisplay();
    }

    for(uint8_t i=0;i<32;i++)
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
        for(uint8_t i=0;i<32;i++)
        {
            Device *d = Device::getDeviceForId(i);
            if(d!=NULL)
                d->printInfo();
        }
        }
    }
    else if(!strcmp(tokens[0],"setname") && tidx > 1)
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
    else if(!strcmp(tokens[0],"delete") && tidx ==2)
    {
        uint8_t id = atoi(tokens[1]);
        Device *d = Device::getDeviceForId(id);
        if(d != NULL)
            delete d;
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
                else if(relay < 1 || relay > NUM_RELAYS)
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
                else if(relay < 1 || relay > NUM_RELAYS)
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
            for(uint8_t i=0;i<32;i++)
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
    else if(!strcmp(tokens[0],"reset"))
    {
        Serial.println(F("Resetting"));
        wdt_enable (WDTO_1S);  // reset after one second, if no "pat the dog" received
        while(1);
    }
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
                lidx = 0;
                handleInput();
                continue;
            }
            line[lidx++] = c;
        }
    }
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
}
