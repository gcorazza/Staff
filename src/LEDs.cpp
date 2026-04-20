//
// Created by gian on 01.03.26.
//

#include "LEDs.h"

CRGB leds[NUM_LEDS];

void setupLEDs() {
    FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);
}

void turnOffAllLEDs() {
    for (int i = 0; i < NUM_LEDS; i++) {
    	leds[i] = CRGB::Black;
  	}
}