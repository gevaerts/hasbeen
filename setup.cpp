#include "Arduino.h"
#include "setup.h"

const uint8_t buttons[]=
{
    A8,A9,A10,A11,A12,A13,A14,A15,
    19,18,17,16,15,14, 24, 25,
    2,3, 4,5, 27,26,29,28,
    31,30,33,32,35,34,37,36,
};

const uint8_t relays[]=
{
    A0,A1,A2,A3,A4,A5,A6,A7,
    38,40,42,44,46,48,50,52,
    39,41,43,45,47,49,51,53
};

const uint8_t pwms[]=
{
    6,7,8,9,10,11,12,13
};

const uint8_t NUM_BUTTONS = ARRAY_SIZE(buttons);
const uint8_t NUM_RELAYS = ARRAY_SIZE(relays);
const uint8_t NUM_PWMS = ARRAY_SIZE(pwms);

