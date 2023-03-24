#include "FastLED.h"
#include "globals.h"
#include "lib.h"

#define DATA_PIN 6

CRGB leds[NUM_LEDS];



int brightness = 0;

void setup() {

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(9600);
  randomSeed(28);
  delay(1000);
  
  clearLEDsInvisible(leds);
}

void loop() {
//  slowLeftRight();
//  lightningStrike();
//   setBulk(leds, 100, 120, CRGB(150,150,150));
  // overlapping();
  // fast();
//   fireWater();
  // FastLED.show();
  fabian();
}







void fabian() {
//    clearLEDsInvisible(leds);
    
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 255, 150);
    brightness = (brightness + 1) % 255;
  }
  
  FastLED.show();
  delay(80);
}
