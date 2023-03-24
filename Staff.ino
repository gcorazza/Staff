#include "FastLED.h"
#include "globals.h"
#include "lib.h"

#define DATA_PIN 6
#define DEL 200;

CRGB leds[NUM_LEDS];

void setup() {

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(9600);
  randomSeed(28);
  delay(100);
}

void loop() {
  policeLights();
}

void policeLights() {
  for (byte i = 1; i <= 2; i++) {
    setPolice(CRGB::Blue, CRGB::Red);
    delay(DEL);
    clearLEDsInvisible(leds);
    delay(DEL);
  }
  delay(DEL);
  for (byte i = 1; i <= 2; i++) {
    setPolice(CRGB::Red, CRGB::Blue);
    delay(DEL);
    clearLEDsInvisible(leds);
    delay(DEL);
  }
  delay(DEL);
}

void setPolice(CRGB c1, CRGB c2) {
  for (byte i = 0; i < NUM_LEDS / 2; i++) {
    leds[i] = c1;
  }
  for (byte i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
    leds[i] = c2;
  }
  FastLED.show();
}
