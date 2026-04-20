#include "PNGAnimation.h"
#include "LEDs.h"
#include <Arduino.h>

// PNG callback functions
static File gFile;

void* myOpen(const char* filename, int32_t* filesize)
{
    gFile = LittleFS.open(filename, "rb");
    if (!gFile) return NULL;
    *filesize = gFile.size();
    return &gFile;
}

void myClose(void* handle){
    File* f = (File*)handle;
    f->close();
}

int32_t myRead(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen){
    File *f = (File *)pFile->fHandle;
    return f->read(pBuf, iLen);
}

int32_t mySeek(PNGFILE *pFile, int32_t iPosition)
{
    File *f = (File *)pFile->fHandle;
    if (!f->seek(iPosition)) return -1;
    return f->position();
}

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

CRGB* PNGAnimation::loopStep() {
    if (finished) return leds;
    int res = png.decode(NULL, 1); // 1 Zeile pro Aufruf
    if (res == 0) { // PNG_DONE == 0 laut PNGdec
        finished = true;
    } else if (res < 0) {
        Serial.println("PNG decode error!");
        finished = true;
    }
    return leds;
}

bool PNGAnimation::isFinished() const {
    return finished;
}

void PNGAnimation::stop() {
    finished = true;
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
    return 1;
}
