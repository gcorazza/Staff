#include "FastLED.h"
#include "globals.h"
#include "lib.h"

#define DATA_PIN 6

CRGB leds[NUM_LEDS];

void setup() {

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(9600);
  randomSeed(28);
  delay(500);
  for (float i = 0; i < 1; i += 1.0 / 20) {
    Serial.print(sqrt(i), 3);
    Serial.print(",");
  }
}

void loop() {
  lightningStrike();
  delay(1000);
}

void fireWater() {
}

void lightningStrike() {
  int from = 0;
  int till = 100;
  int widthFront = 2;

  int widthBack = 10;
  CRGB color = CRGB::Aqua;
  int durationMs = 1000;

  long start = millis();
  long now = millis();

  float modifier[20] = { 0.000, 0.224, 0.316, 0.387, 0.447, 0.500, 0.548, 0.592, 0.632, 0.671, 0.707, 0.742, 0.775, 0.806, 0.837, 0.866, 0.894, 0.922, 0.949, 0.975 };

  while (now - start < durationMs) {
    double framePercent = ((double)(now - start)) / durationMs;
    clearLEDsInvisible(leds);
    lightningFrame(leds, from, till, widthFront, widthBack, color, frameModifier(modifier, 20, framePercent));
    FastLED.show();
    now = millis();
  }
  clearLEDsInvisible(leds);
  FastLED.show();
}

void ether() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(200, 0, 200) % random(0, 100);
  }
  FastLED.show();
  delay(100);
}