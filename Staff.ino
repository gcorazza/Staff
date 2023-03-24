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
//  slowLeftRight();
//  lightningStrike();
   setBulk(leds, 100, 120, CRGB(150,150,150));
  // overlapping();
  // fast();
  // fireWater();
  // FastLED.show();
}

float sin01(float x) {
  return ((sin(x)) / (2.8)) + 0.5;
}


void slowLeftRight() {

  long start = millis();
  long now = millis();

  int durationMs = 4000;

  while (now - start < durationMs) {

    double framePercent = ((double)(now - start)) / durationMs;
    CRGB ledsg[NUM_LEDS];
    clearLEDsInvisible(leds);

    lightningFrame(leds, 0, 120, 40, 40, CRGB::Red, abs(framePercent * 2 - 1)*0.8);
    FastLED.show();
    now = millis();
  }
  FastLED.show();
}

void overlapping() {

  float e = 2.71828182;

  long start = millis();
  long now = millis();

  int durationMs = 20000;

  while (now - start < durationMs) {

    double framePercent = ((double)(now - start)) / durationMs;
    CRGB ledsg[NUM_LEDS];
    CRGB ledsb[NUM_LEDS];
    clearLEDsInvisible(leds);
    clearLEDsInvisible(ledsg);
    clearLEDsInvisible(ledsb);

    lightningFrame(leds, 0, 120, 40, 40, CRGB::Red, sin01(framePercent * 10));
    lightningFrame(ledsg, 0, 120, 40, 40, CRGB::Green, sin01((framePercent + 4) * 8));
    lightningFrame(ledsb, 0, 120, 20, 20, CRGB::Blue, sin01((framePercent + 6) * 12));
    addToFirst(leds, ledsg);
    addToFirst(leds, ledsb);
    FastLED.show();
    now = millis();
  }
  // clearLEDsInvisible(leds);
  FastLED.show();
}

void fast() {
  int widthFront = 10;
  int widthBack = 10;
  int durationMs = 2000;

  float e = 2.71828182;

  long start = millis();
  long now = millis();

  while (now - start < durationMs) {
    double framePercent = ((double)(now - start)) / durationMs;
    clearLEDsInvisible(leds);
    lightningFrame(leds, 120, 0, widthFront, widthBack, CRGB::Pink, ((sin(pow(e, (framePercent * 3.3 + 0.5)))) / (2.5)) + 0.5);
    FastLED.show();
    now = millis();
  }
  clearLEDsInvisible(leds);
  FastLED.show();
}

void fireWater() {

  int widthFront = 10;
  int widthBack = 10;
  int durationMs = 10000;

  long start = millis();
  long now = millis();

  while (now - start < durationMs) {
    double framePercent = ((double)(now - start)) / durationMs;
    clearLEDsInvisible(leds);
    CRGB leds2[NUM_LEDS];
    CRGB leds3[NUM_LEDS];
    CRGB leds4[NUM_LEDS];
    lightningFrame(leds, 0, 62, widthFront, widthBack, CRGB::Red, framePercent);
    lightningFrame(leds2, 120, 58, widthFront, widthBack, CRGB::Blue, framePercent);
    addToFirst(leds, leds2);
    FastLED.show();
    now = millis();
  }
  clearLEDsInvisible(leds);
  FastLED.show();
}

void lightningStrike() {
  int widthFront = 2;
  int widthBack = 10;
  int durationMs = 1000;

  long start = millis();
  long now = millis();

  float modifier[20] = { 0, 0.4169913895607595, 0.5618039271819768, 0.6578185439802253, 0.7283221409124393, 0.7826476292961627, 0.825613063541047, 0.860049404366313, 0.8878708941948211, 0.910247022889621, 0.927364489471808, 0.9410178852180805, 0.9524824099642146, 0.9622535793171002, 0.9706587718820873, 0.9779081236478762, 0.9841452330383459, 0.9894613822283056, 0.9938974691287625, 0.9974456444941892 };

  while (now - start < durationMs) {
    double framePercent = ((double)(now - start)) / durationMs;
    clearLEDsInvisible(leds);
    lightningFrame(leds, 0, 100, widthFront, widthBack, CRGB::Aqua, frameModifier(modifier, 20, framePercent) * 0.9);
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
