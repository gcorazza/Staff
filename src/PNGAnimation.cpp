#include "PNGAnimation.h"
#include "LEDs.h"
#include <Arduino.h>

extern void* myOpen(const char* filename, int32_t* filesize);
extern void myClose(void* handle);
extern int32_t myRead(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen);
extern int32_t mySeek(PNGFILE *pFile, int32_t iPosition);
extern int myDraw(PNGDRAW* draw);

PNGAnimation* PNGAnimation::instance = nullptr;

PNGAnimation::PNGAnimation(const char* filename, CRGB* leds, uint16_t numLeds)
    : leds(leds), numLeds(numLeds), filename(filename), finished(false), decodeState(0) {
    instance = this;
    int res = png.open(filename, myOpen, myClose, myRead, mySeek, myDraw);
    if (res != PNG_SUCCESS) {
        Serial.println("PNG open failed!");
        finished = true;
        return;
    }
    if (png.getWidth() != numLeds) {
        Serial.print("PNG width = ");
        Serial.print(png.getWidth());
        Serial.print(" != NUM_LEDS = ");
        Serial.println(numLeds);
        finished = true;
        png.close();
        return;
    }
}

PNGAnimation::PNGAnimation(uint8_t* buffer, size_t length, CRGB* leds, uint16_t numLeds)
    : buffer(buffer), bufferLength(length), leds(leds), numLeds(numLeds), finished(false), decodeState(0), useBuffer(true) {
    instance = this;
    int res = png.openRAM(buffer, length, myDraw);
    if (res != PNG_SUCCESS) {
        Serial.println("PNG open failed!");
        finished = true;
        return;
    }
    if (png.getWidth() != numLeds) {
        Serial.print("PNG width = ");
        Serial.print(png.getWidth());
        Serial.print(" != NUM_LEDS = ");
        Serial.println(numLeds);
        finished = true;
        png.close();
        return;
    }
}

PNGAnimation::~PNGAnimation() {
    png.close();
    if (!useBuffer && gFile) gFile.close();
    instance = nullptr;
}

void PNGAnimation::loopStep() {
    if (finished) return;
    int res = png.decode(NULL, 1); // 1 Zeile pro Aufruf
    if (res == 0) { // PNG_DONE == 0 laut PNGdec
        finished = true;
    } else if (res < 0) {
        Serial.println("PNG decode error!");
        finished = true;
    }
}

bool PNGAnimation::isFinished() const {
    return finished;
}

int PNGAnimation::myDraw(PNGDRAW* draw) {
    if (!instance) return 0;
    uint8_t* p = draw->pPixels;
    for (int x = 0; x < draw->iWidth; x++) {
        uint8_t r = p[x * 3 + 0];
        uint8_t g = p[x * 3 + 1];
        uint8_t b = p[x * 3 + 2];
        instance->leds[x] = CRGB(r, g, b);
    }
    FastLED.show();
    return 1;
}
