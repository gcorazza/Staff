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
  lightningStrike();
  delay(1000);
}

void fireWater() {
}

void lightningStrike() {
  int from = 0;
  int till = 120;
  int widthFront = 2;
  int widthBack = 10;
  CRGB color = CRGB::Aqua;
  int durationMs =100;
  long start = millis();
  long now = millis();

  while (now - start < durationMs) {
    double framePercent = ((double)(now - start)) / durationMs;
    CRGB leds2[NUM_LEDS];
    lightningFrame(leds, from, till, widthFront, widthBack, color, framePercent);
    lightningFrame(leds, from, till, widthFront, widthBack, color, framePercent);
    FastLED.show();
    now = millis();
  }
  clearLEDsInvisible(leds);
  FastLED.show();
}