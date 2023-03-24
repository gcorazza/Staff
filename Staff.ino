#include "FastLED.h"
#include "globals.h"
#include "lib.h"

#define DATA_PIN 6

CRGB leds[NUM_LEDS];

void setup() {

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(9600);
  randomSeed(28);
  delay(1000);
}

void loop() {
  
}
