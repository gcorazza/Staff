#include "HardwareSerial.h"
#include "FastLED.h"
#include "globals.h"


void clearLEDsInvisible(CRGB leds[]) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  FastLED.show();
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


template<typename TwoD>
void animate(CRGB leds[], int amount, TwoD& scene, int rows, int cols, float sec) {
  long startTime = millis();
  float percentTime = 0;
  while (percentTime < 1) {
    percentTime = ((float)(millis() - startTime)) / (sec * 1000);
    if (percentTime > 1) {
      percentTime = 1;
    }
    setFrame(leds, amount, scene, rows, cols, percentTime);
    FastLED.show();
    // delay(10000000);
  }
}

template<typename TwoD>
void setFrame(CRGB leds[], int amount, TwoD& scene, int rows, int cols, float percentTime) {
  int tly = floor(percentTime * (rows - 1));

  for (int x = 0; x < amount; x++) {
    float percentScene = (float)(x) / amount;

    int tlx = floor(percentScene * (cols - 1));

    float wx = 1.0 / (cols - 1);
    float wy = 1.0 / (rows - 1);
    float sx = tlx * wx;
    float sy = tly * wy;
    float xP = (percentScene - sx) / wx;
    float yP = (percentTime - sy) / wy;

    float inter = interpolate(scene[tlx][tly], scene[tlx + 1][tly],
                              scene[tlx][tly + 1], scene[tlx + 1][tly + 1],
                              xP, yP);
    int rgb = (int)(inter * inter * inter * 255);
    leds[x] = CRGB(rgb, rgb, rgb);

    // Serial.println("---->");
    // Serial.println(percentTime);
    // Serial.print(x);
    // Serial.print(":");
    // Serial.println(inter);
    // Serial.println("<----");
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

void lightningFrame(CRGB leds[], int from, int till, int widthFront, int widthBack, CRGB color, float framePercent) {
  int widthLeft;
  int widthRight;

  int from_;
  int till_;

  if (from > till) {
    widthLeft = widthFront;
    widthRight = widthBack;

    from_ = from + widthLeft;
    till_ = till - widthRight;
  } else {
    widthLeft = widthBack;
    widthRight = widthFront;

    from_ = from - widthRight;
    till_ = till + widthLeft;
    // Serial.println(till_);
  }

  float at = from_ + (till_ - from_) * framePercent;

  // Serial.println(framePercent);
  // Serial.println((int)at);


  float ml = 1.0 / widthLeft;
  float mr = 1.0 / widthRight;

  for (float ati = at - widthLeft; ati <= at + widthRight + 1; ati++) {
    if (min(from, till) < ati && min(from, till) + abs(from - till) > ati) {
      int absAtI = (int)ati;
      float distance = abs(absAtI - at);
      float h = absAtI < at ? 1 - (distance * ml) : 1 - (distance * mr);
      h = h < 0 ? 0 : h;
      // Serial.println("h");
      // Serial.println(h);
      setChecked(leds, absAtI, color % (h * h * h * 100));
    }
  }
}

bool isBlack(CRGB color) {
  return color.r == 0 && color.g == 0 && color.b == 0;
}

void addToFirst(CRGB leds1[], CRGB leds2[]) {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (isBlack(leds1[i])) {
      leds1[i] = leds2[i];
    } else if (isBlack(leds2[i])) {
    } else {
      leds1[i] = CRGB(
        (leds1[i].r + leds2[i].r) / 2,
        (leds1[i].g + leds2[i].g) / 2,
        (leds1[i].b + leds2[i].b) / 2);
    }
  }
}

float frameModifier(float points[], int size, float percentTime) {
  // Serial.println("---->");
  int left = floor(percentTime * (size - 1));
  // Serial.println(percentTime);
  int right = left + 1;

  float wx = 1.0 / (size - 1);
  float sx = left * wx;
  float xP = (percentTime - sx) / wx;
  // Serial.println(xP);
  // Serial.println(left);
  // Serial.println(points[left] + ((points[right] - points[left]) * xP));

  return points[left] + ((points[right] - points[left]) * xP);
}
