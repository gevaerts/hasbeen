#ifndef SETUP_H
#define SETUP_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define NUM_PWMS 12
#define NUM_BUTTONS 48

extern const uint8_t buttons[NUM_BUTTONS];
extern const uint8_t pwms[NUM_PWMS];

#define PRESSED 0
#define RELEASED 1

#endif
