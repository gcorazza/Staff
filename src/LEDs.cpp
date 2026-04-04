//
// Created by gian on 01.03.26.
//

#include "LEDs.h"
#include "lib.h"

CRGB leds[NUM_LEDS];

void setupLEDs() {
    FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);
}

void turnOffAllLEDs() {
    setBulk(leds, 0, NUM_LEDS, CRGB(0,0,0));
    FastLED.show();
}


