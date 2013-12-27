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

int buttonState[32]; // NUM_BUTTONS
int buttonCount[32]; // NUM_BUTTONS
unsigned int iteration;
unsigned int lastChange[32]; // NUM_BUTTONS

void setup()
{
    Serial.begin(9600);
    Serial.println(F("Init begin"));
    for(int i=0;i<32;i++)
    {
        Device::restore(i);
    }
    for(int i=0;i<ARRAY_SIZE(buttonState);i++)
    {
        pinMode(buttons[i],INPUT);
        digitalWrite(buttons[i],HIGH); // Enable pull-up

        buttonState[i] = RELEASED;
    }

    Serial.println(F("Init done"));
}

char line[64];
char lidx = 0;

void handleInput()
{
    char *tokens[10];
    char *t;
    int tidx = 0;
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
        for(int i=0;i<32;i++)
        {
            Device *d = Device::getDeviceForId(i);
            if(d!=NULL)
                d->printInfo();
        }
    }
    else if(!strcmp(tokens[0],"setname") && tidx > 1)
    {
        int id = atoi(tokens[1]);
        char name[16]="";
        for(int i=2;i<tidx;i++)
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
        int id = atoi(tokens[1]);
        Device *d = Device::getDeviceForId(id);
        if(d != NULL)
            delete d;
    }
    else if(!strcmp(tokens[0],"define") && tidx > 1)
    {
        if(!strcmp(tokens[1],"dimmer"))
        {
            if(tidx != 8)
            {
                Serial.println(F("define dimmer <id> <board> <relay> <buttonPlus> <buttonMin> <pwm>"));
            }
            else
            {
                int id, board, relay, buttonPlus, buttonMin, pwm;
                id = atoi(tokens[2]);
                board = atoi(tokens[3]);
                relay = atoi(tokens[4]);
                buttonPlus = atoi(tokens[5]);
                buttonMin = atoi(tokens[6]);
                pwm = atoi(tokens[7]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial.println(F("id not empty"));
                else if(relay < 0 || relay >= NUM_RELAYS)
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
                    Device *d = new Dimmer(id, board, relay,buttonPlus,buttonMin,pwm);
                    d->printInfo();
                }
            }
        }
        else if(!strcmp(tokens[1],"lightpoint"))
        {
            if(tidx != 6)
            {
                Serial.println(F("define lightpoint <id> <board> <relay> <button>"));
            }
            else
            {
                int id, board, relay, button;
                id = atoi(tokens[2]);
                board = atoi(tokens[3]);
                relay = atoi(tokens[4]);
                button = atoi(tokens[5]);

                if(id < 0 || id >= NUM_DEVICES)
                    Serial.println(F("id out of range"));
                else if(Device::getDeviceForId(id) != NULL)
                    Serial.println(F("id not empty"));
                else if(relay < 0 || relay >= NUM_RELAYS)
                    Serial.println(F("relay out of range"));
                else if(button < 0 || button >= NUM_BUTTONS)
                    Serial.println(F("button out of range"));
                else if(Device::getDeviceForButton(button) != NULL)
                    Serial.println(F("button already in use"));
                else
                {
                    Device *d = new Lightpoint(id, board, relay,button);
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
                int id, address;
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
            for(int i=0;i<32;i++)
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
            int id = atoi(tokens[1]);
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

int status = 0;
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

    int i;
    for(i=0;i<NUM_BUTTONS;i++)
    {
        int value = digitalRead(buttons[i]);

        int changed = 0;
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
                    int previous = buttonState[i];
                    if(changed)
                        previous = !buttonState[i];

                    d->press(i,previous);
                }
            }
        }
    }
}
