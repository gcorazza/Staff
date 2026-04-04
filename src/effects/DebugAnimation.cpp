#include "DebugAnimation.h"
#include "../LEDs.h"
#include <FastLED.h>

DebugAnimation::DebugAnimation(CRGB* leds, uint16_t numLeds)
    : leds(leds), numLeds(numLeds) {
}

//50 von oben
//64 von unten

void DebugAnimation::animationStart() {

}

void DebugAnimation::loopStep() {

    for (uint16_t i = 0; i < numLeds; ++i) {
        switch (i % 3) {
            case 0: leds[i] = CRGB::Red; break;
            case 1: leds[i] = CRGB::Green; break;
            case 2: leds[i] = CRGB::Blue; break;
        }
    }

    if (millis() / 1000 % 2 == 0 ) {
        leds[0] = CRGB::White;
        leds[NUM_LEDS-1] = CRGB::White;
    }

}

bool DebugAnimation::isFinished() const { return false; }
bool DebugAnimation::isInterruptible() const { return true; }
