//
// Created by gian on 01.03.26.
//

#ifndef STAFF_LEDS_H
#define STAFF_LEDS_H

#include <Arduino.h>
#include <FastLED.h>
#include "src/globals.h"


#define LED_DATA_PIN 23

extern CRGB leds[NUM_LEDS];

void setupLEDs();
void turnOffAllLEDs();

#endif // STAFF_LEDS_H