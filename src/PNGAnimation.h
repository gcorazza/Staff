// PNGAnimation.h
#ifndef STAFF_PNGANIMATION_H
#define STAFF_PNGANIMATION_H

#include "src/effects/Animation.h"
#include <PNGdec.h>
#include <LittleFS.h>
#include <FastLED.h>

// PNG callback functions
void* myOpen(const char* filename, int32_t* filesize);
void myClose(void* handle);
int32_t myRead(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen);
int32_t mySeek(PNGFILE *pFile, int32_t iPosition);

class PNGAnimation : public Animation {
public:
    PNGAnimation(const char* filename, CRGB* leds, uint16_t numLeds);
    PNGAnimation(uint8_t* buffer, size_t length, CRGB* leds, uint16_t numLeds);
    ~PNGAnimation();
    CRGB* loopStep() override;
    bool isFinished() const override;
    bool isInterruptible() const override { return true; }
    void stop() override;
	void animationStart() override {};
private:
    PNG png;
    File gFile;
    uint8_t* buffer = nullptr;
    size_t bufferLength = 0;
    CRGB* leds;
    uint16_t numLeds;
    String filename;
    bool finished;
    int decodeState;
    bool useBuffer = false;
    static int myDraw(PNGDRAW* draw);
    static PNGAnimation* instance;
};

#endif // STAFF_PNGANIMATION_H

