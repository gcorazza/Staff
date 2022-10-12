#include "HardwareSerial.h"
#include "FastLED.h"
#include "globals.h"


void clearLEDsInvisible() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

float interpolate(float tl, float tr, float bl, float br, float xP, float yP) {
  // Serial.println("->");
  // Serial.println(tl);
  // Serial.println(tr);
  // Serial.println(bl);
  // Serial.println(br);
  // Serial.println(xP);
  // Serial.println(yP);
  // Serial.println("<-");
  float tltr = tl - (tl - tr) * xP;
  float blbr = bl - (bl - br) * xP;
  return blbr - (blbr - tltr) * (1 - yP);
}


// template<typename TwoD>
// void animate(CRGB leds[], int amount, TwoD& scene, int rows, int cols, float sec) {
//   long startTime = millis();
//   float percentTime = 0;
//   while (percentTime < 1) {
//     percentTime = ((float)(millis() - startTime)) / (sec * 1000);
//     if (percentTime > 1) {
//       percentTime = 1;
//     }
//     setFrame(leds, amount, scene, rows, cols, percentTime);
//     FastLED.show();
//     // delay(10000000);
//   }
// }

// template<typename TwoD>
// void setFrame(CRGB leds[], int amount, TwoD& scene, int rows, int cols, float percentTime) {
//   int tly = floor(percentTime * (rows - 1));

//   for (int x = 0; x < amount; x++) {
//     float percentScene = (float)(x) / amount;

//     int tlx = floor(percentScene * (cols - 1));

//     float wx = 1.0 / (cols - 1);
//     float wy = 1.0 / (rows - 1);
//     float sx = tlx * wx;
//     float sy = tly * wy;
//     float xP = (percentScene - sx) / wx;
//     float yP = (percentTime - sy) / wy;

//     float inter = interpolate(scene[tlx][tly], scene[tlx + 1][tly],
//                               scene[tlx][tly + 1], scene[tlx + 1][tly + 1],
//                               xP, yP);
//     int rgb = (int)(inter * inter * inter * 255);
//     leds[x] = CRGB(rgb, rgb, rgb);

//     // Serial.println("---->");
//     // Serial.println(percentTime);
//     // Serial.print(x);
//     // Serial.print(":");
//     // Serial.println(inter);
//     // Serial.println("<----");
//   }
// }


// void test(int amount) {

//   float scene[5][3]{
//     { 0, 1, 0 },
//     { 0, 1, 0 },
//     { 0, 1, 0 },
//     { 0, 1, 0 },
//     { 0, 1, 0 },
//   };
//   animate( amount, scene, 3, 5, 1);
// }

void setChecked(int i, CRGB color) {
  if (i < 0 || i >= NUM_LEDS) {
    return;
  }
  leds[i] = color;
}

void lightning(CRGB leds[], int from, int till, int widthFront, int widthBack, CRGB color, float framePercent) {
  clearLEDsInvisible();

  int widthLeft;
  int widthRight;

  int from_;
  int till_;

  if (from > till) {
    widthLeft = widthBack;
    widthRight = widthFront;
  } else {
    widthLeft = widthFront;
    widthRight = widthBack;
  }

  float at = from + (till - from) * framePercent;

  // Serial.println(framePercent);
  // Serial.println((int)at);


  float ml = 1.0 / widthLeft;
  float mr = 1.0 / widthRight;

  for (float ati = at - widthLeft; ati <= at + widthRight + 1; ati++) {
    int absAtI = (int)ati;
    float distance = abs(absAtI - at);
    float h = absAtI < at ? 1 - (distance * ml) : 1 - (distance * mr);
    h = h < 0 ? 0 : h;
    // Serial.println("h");
    // Serial.println(h);
    setChecked(absAtI, color % (h * h * h * 100));
  }
}

void animateLightning(CRGB leds[], int from, int till, int widthFront, int widthBack, CRGB color, long durationMs) {
  long start = millis();
  long now = millis();

  while (now - start < durationMs) {
    // Serial.println("---");

    // Serial.println(durationMs);
    // Serial.println(((now - start)));
    // double t = 1.0 * (now - start);
    // Serial.println(t, 3);

    double framePercent = ((double)(now - start)) / durationMs;
    // Serial.println(framePercent, 3);
    lightning(leds, from, till, widthFront, widthBack, color, framePercent);
    FastLED.show();
    now = millis();
  }
}