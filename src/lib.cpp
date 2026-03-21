#include "HardwareSerial.h"
#include "FastLED.h"
#include "globals.h"


void clearLEDsInvisible(CRGB leds[]) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}


void setChecked(CRGB leds[], int i, CRGB color) {
  if (i < 0 || i >= NUM_LEDS) {
    return;
  }
  leds[i] = color;
}

void setBulk(CRGB leds[], int from, int to, CRGB color) {
  for (int i = from; i <= to; i++) {
    setChecked(leds, i, color);
  }
}
