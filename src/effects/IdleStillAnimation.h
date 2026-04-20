//
// Created by gian on 20.04.26.
//

#ifndef STAFF_IDLESTILLANIMATION_H
#define STAFF_IDLESTILLANIMATION_H

#include <Arduino.h>
#include <FastLED.h>
#include "Animation.h"

class IdleStillAnimation : public Animation {
public:
    IdleStillAnimation(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive);
    ~IdleStillAnimation();

    // Animation interface
    CRGB* loopStep() override;
    bool isFinished() const override;
    bool isInterruptible() const override { return true; }
    void stop() override;

    void beginBurst(unsigned long now);
    bool isBurstActive() const { return burstActive; }
    void render(unsigned long now);

private:
    struct LedState {
        bool active;
        unsigned long startTime;
        unsigned long duration;
        CRGB currentColor;
        uint8_t currentBrightness;
        bool bursting;
        float burstRadius;
        float burstMaxRadius;
    };

    void runIdleFrame(unsigned long now);
    void runBurstFrame(unsigned long now);
    void startLed(uint16_t index, unsigned long now);
    uint8_t brightnessFromColor(const CRGB& color) const;
    uint8_t computeBurstRange(uint8_t brightness) const;
    void setLedChecked(float index, const CRGB& color);
    CRGB applyBrightness(const CRGB& color, float scale);

    CRGB* leds;
    uint16_t numLeds;
    uint8_t ledsAlive;
    LedState* states;

    bool burstActive;
    unsigned long lastBurstUpdate;
    bool shouldStop;
    bool hasStopped;
};

#endif // STAFF_IDLESTILLANIMATION_H
