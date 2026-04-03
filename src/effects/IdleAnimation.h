//
// Created by gian on 30.03.26.
//

#ifndef STAFF_IDLEANIMATION_H
#define STAFF_IDLEANIMATION_H

#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include "../StickGesture.h"

class IdleAnimation {
public:
    IdleAnimation(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive);
    ~IdleAnimation();

    void updateMovementState(StickGesture::MovementState newState);
    void idleAnimation();

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
};

#endif // STAFF_IDLEANIMATION_H

