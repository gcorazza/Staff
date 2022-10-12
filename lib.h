#include "FastLED.h"

void lightningFrame(CRGB leds[], int from, int till, int widthFront, int widthBack, CRGB color, float framePercent);
void clearLEDsInvisible(CRGB leds[]);
void addToFirst(CRGB leds1[], CRGB leds2[]);