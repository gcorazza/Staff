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
  delay(1000);
}

void loop() {
  // lightningStrike();
  fast();
  delay(1000);
}

void fast(){
  float modifier[40] = {0, 0.16796758479206453, 0.33593516958412906, 0.4731369495215315, 0.5933248831192569, 0.7135128167169823, 0.7934130672683462, 0.8509559908187263, 0.9084989143691066, 0.9660418379194868, 0.9828271670923865, 0.9409282904635601, 0.8990294138347338, 0.8571305372059075, 0.805114119180385, 0.6318839726721119, 0.4586538261638386, 0.31536195112903953, 0.18058481592552347, 0.04887447250564444, 0.07153368675776232, 0.09419290100988026, 0.11685211526199821, 0.13951132951411616, 0.33214244908172796, 0.6122365173062504, 0.9015229764426426, 0.8916233013880492, 0.743674122778789, 0.4796443410669312, 0.20622360614884133, 0.17560855379156656, 0.1449935014342918, 0.5878014249272033, 0.882880105633735, 0.4719637698718202, 0.1413276580655941, 0.30416521605574454, 0.7567642380650128, 0.36448685906553446};

  int widthFront = 10;
  int widthBack = 10;
  int durationMs = 2000;

  float e = 2.71828182;

  long start = millis();
  long now = millis();

  while (now - start < durationMs) {
    double framePercent = ((double)(now - start)) / durationMs;
    clearLEDsInvisible(leds);
    lightningFrame(leds, 120, 0, widthFront, widthBack, CRGB::White, ((sin(pow(e,(framePercent*3.3+0.5))))/(2.5))+0.5);
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

  float modifier[20] = {0, 0.4169913895607595, 0.5618039271819768, 0.6578185439802253, 0.7283221409124393, 0.7826476292961627, 0.825613063541047, 0.860049404366313, 0.8878708941948211, 0.910247022889621, 0.927364489471808, 0.9410178852180805, 0.9524824099642146, 0.9622535793171002, 0.9706587718820873, 0.9779081236478762, 0.9841452330383459, 0.9894613822283056, 0.9938974691287625, 0.9974456444941892};

  while (now - start < durationMs) {
    double framePercent = ((double)(now - start)) / durationMs;
    clearLEDsInvisible(leds);
    lightningFrame(leds, 0, 100, widthFront, widthBack, CRGB::Aqua, frameModifier(modifier, 20, framePercent));
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