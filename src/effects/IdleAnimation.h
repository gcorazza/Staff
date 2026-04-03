//
// Created by gian on 30.03.26.
//

#ifndef STAFF_IDLEANIMATION_H
#define STAFF_IDLEANIMATION_H

#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include "../StickGesture.h"
#include "Animation.h"

class IdleAnimation : public Animation {
public:
    IdleAnimation(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive, StickGesture* gesture);
    ~IdleAnimation();

    void idleAnimation();
    void loopStep() override { idleAnimation(); }
    bool isFinished() const override { return false; }
    bool isInterruptible() const override { return true; }

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

    void updateMovementState(StickGesture::MovementState newState);
    void startLed(uint16_t index, unsigned long now);
    void runIdleFrame(unsigned long now);
    void runBurstFrame(unsigned long now);
    void beginBurst(unsigned long now);
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
    StickGesture::MovementState currentMovementState;
    StickGesture::MovementState previousMovementState;
    StickGesture* gesture;
};

#endif // STAFF_IDLEANIMATION_H

