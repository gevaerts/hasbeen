#include "Arduino.h"
#include "setup.h"
#include "lightpoint.h"
#include "dimmer.h"

#define STATUS1 22
#define STATUS2 23


Dimmer dimmers[] =
{
    Dimmer(0,0,1,0),
    Dimmer(1,2,3,1),
    Dimmer(2,4,5,2),
    Dimmer(3,6,7,3),
    Dimmer(4,8,9,4),
    Dimmer(5,10,11,5),
    Dimmer(6,12,13,6),
    Dimmer(7,14,15,7),
};
Lightpoint lightpoints[] =
{
    Lightpoint(8,16),
    Lightpoint(9,17),
    Lightpoint(10,18),
    Lightpoint(11,19),
    Lightpoint(12,20),
    Lightpoint(13,21),
    Lightpoint(14,22),
    Lightpoint(15,23),
    Lightpoint(16,24),
    Lightpoint(17,25),
    Lightpoint(18,26),
    Lightpoint(19,27),
    Lightpoint(20,28),
    Lightpoint(21,29),
    Lightpoint(22,30),
    Lightpoint(23,31)
};

Relay *lightpointsByButton[32]; // NUM_BUTTONS
int buttonState[32]; // NUM_BUTTONS
int buttonCount[32]; // NUM_BUTTONS
unsigned int iteration;
unsigned int lastChange[32]; // NUM_BUTTONS

void setup()
{
    Serial.begin(9600);
    Serial.println("Init begin");
    int i,j,k;
    for(i=0;i<ARRAY_SIZE(lightpointsByButton);i++)
    {
        for(j=0;j<ARRAY_SIZE(lightpoints);j++)
        {
            if(lightpoints[j].respondsToButton(i))
            {
                if(lightpointsByButton[i] != NULL)
                {
                    // Error!
                }
                lightpointsByButton[i] = &lightpoints[j];
            }
        }
        for(j=0;j<ARRAY_SIZE(dimmers);j++)
        {
            if(dimmers[j].respondsToButton(i))
            {
                if(lightpointsByButton[i] != NULL)
                {
                    // Error!
                }
                lightpointsByButton[i] = &dimmers[j];
            }
        }
    }
    for(i=0;i<ARRAY_SIZE(buttonState);i++)
    {
        pinMode(buttons[i],INPUT);
        digitalWrite(buttons[i],HIGH); // Enable pull-up

        buttonState[i] = RELEASED;
    }

    // Startup animation
#if 0
    for(k=0;k<2;k++)
    {
        for(j=0;j<NUM_RELAYS;j+=8)
        {
            for(i=j;i<j+8 && i<NUM_RELAYS;i++)
                digitalWrite(relays[i],HIGH);
            delay(500);
            for(i=j;i<j+8 && i<NUM_RELAYS;i++)
                digitalWrite(relays[i],LOW);
        }
    }
    for(k=0;k<3;k++)
        for(i=0;i<NUM_RELAYS;i++)
        {
            digitalWrite(relays[i],HIGH);
            delay(10);
            digitalWrite(relays[i],LOW);
        }
#endif
    Serial.println("Init done");
}

int status = 0;
void loop()
{
    status = !status;
    pinMode(STATUS1,OUTPUT);
    pinMode(STATUS2,OUTPUT);
    digitalWrite(STATUS1,status);
    digitalWrite(STATUS2,!status);

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
                Serial.print("Button ");
                Serial.print(i,DEC);
                Serial.println(" pressed");
                if(lightpointsByButton[i] != NULL)
                {
                    int previous = buttonState[i];
                    if(changed)
                        previous = !buttonState[i];

                    lightpointsByButton[i]->press(i,previous);
                }
            }
        }
    }
}
