#ifndef SETUP_H
#define SETUP_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

extern const uint8_t buttons[];
extern const uint8_t relays[];
extern const uint8_t pwms[];

extern const uint8_t NUM_BUTTONS;
extern const uint8_t NUM_RELAYS;
extern const uint8_t NUM_PWMS;

#define PRESSED 0
#define RELEASED 1

#endif
