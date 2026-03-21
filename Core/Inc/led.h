#ifndef LED_H
#define LED_H

#include "main.h"

typedef enum {
  LED_1 = 0,
  LED_2,
  LED_3,
  LED_4,
  LED_5,
  LED_6,
  LED_7,
  LED_COUNT
} LedIndex;

void AllOn(void);
void AllOff(void);
void LedFlash(LedIndex index, int times, int delay);
void LedSet(LedIndex index, GPIO_PinState state);
void LedToggle(LedIndex index);

#endif /* LED_H */