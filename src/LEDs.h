//
// Created by gian on 01.03.26.
//

#ifndef STAFF_LEDS_H
#define STAFF_LEDS_H

#include <Arduino.h>
#include <FastLED.h>
#include <PNGdec.h>
#include <LittleFS.h>
#include "src/globals.h"


#define LED_DATA_PIN 23

extern File gFile;
extern CRGB leds[NUM_LEDS];
extern PNG png;

bool drawPNGtoLEDs(const char* filename);
bool drawPNGtoLEDs(uint8_t* buffer, size_t length);
void setupLEDs();

#endif // STAFF_LEDS_H