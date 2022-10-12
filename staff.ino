#include "FastLED.h"
#include "globals.h"
#include "lib.h"

#define DATA_PIN 6

CRGB leds[NUM_LEDS];

void setup() {

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(9600);
  delay(500);
}

void loop() {
  animateLightning(leds, 0, 120, 10, 3, CRGB(200,50,0), 100000);
  delay(1000);
}

void fireWater() {
}
