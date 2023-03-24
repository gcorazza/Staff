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
  policeLights();
}

void policeLights()
{
  setPolice();
  delay(100);
  clearLEDsInvisible(leds);
  delay(100);
  FastLED.show();
}

void setPolice()
{
for(byte i = 0; i < NUM_LEDS / 2; i++)
  {
    leds[i] = CRGB::Blue;
  }
  for(byte i = NUM_LEDS / 2; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Red;
  }
}
