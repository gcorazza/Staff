#include <Arduino.h>
#include "FastLED.h"
#include "globals.h"
#include "lib.h"
#include "GY521.h"

#define DATA_PIN 23

int i=0;

CRGB leds[NUM_LEDS];

void setup() {

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(115200);
  randomSeed(28);
  delay(1000);
  setupGy();

}

void loop() {
  loopGy();
}

