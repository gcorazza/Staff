#ifndef STAFF_DEBUGANIMATION_H
#define STAFF_DEBUGANIMATION_H

#include "Animation.h"
#include <FastLED.h>

class DebugAnimation : public Animation {
public:
    DebugAnimation(CRGB* leds, uint16_t numLeds);
    void loopStep() override;
    bool isFinished() const override;
    bool isInterruptible() const override;
    void animationStart() override;
private:
    CRGB* leds;
    uint16_t numLeds;
    bool didStartEffect = false;
};

#endif // STAFF_DEBUGANIMATION_H

