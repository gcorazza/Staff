#include "FastLED.h"
#include "globals.h"
#include "lib.h"

#define DATA_PIN 6

CRGB leds[NUM_LEDS];

// struct point {
//     float x;
//     float y;
// };

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

  float modifier[20] = { 0, 0.33258406709444666, 0.46490449675653306, 0.56049733416016, 0.6355417319472685, 0.696654350826582, 0.7475876560916674, 0.7908562382762063, 0.827911572318772, 0.8597815366570001, 0.8882739940578332, 0.9162339729036082, 0.9405264977383813, 0.9606916028556446, 0.9763277881072331, 0.9877314636749639, 0.9950112454623635, 0.9983366463800426, 0.9987308766426587, 0.9981819327476077 };

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