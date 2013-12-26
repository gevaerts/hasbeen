#ifndef SETUP_H
#define SETUP_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

extern const int buttons[];
extern const int relays[];
extern const int pwms[];

extern const int NUM_BUTTONS;
extern const int NUM_RELAYS;
extern const int NUM_PWMS;

#define PRESSED 0
#define RELEASED 1

#endif
