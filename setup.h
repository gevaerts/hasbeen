#ifndef SETUP_H
#define SETUP_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define NUM_PWMS 12
#define NUM_BUTTONS 50
#define DEVICE_EEPROM_BASE 128
#define ALIAS_EEPROM_BASE 0
#define NUM_DEVICES 64

extern const uint8_t buttons[NUM_BUTTONS];
extern const uint8_t pwms[NUM_PWMS];

#define PRESSED 0
#define RELEASED 1

uint8_t buttonStatus(uint8_t button);
extern uint8_t verbose;

#endif
