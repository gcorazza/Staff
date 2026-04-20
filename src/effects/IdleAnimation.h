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
#include "IdleStillAnimation.h"
#include "IdleMovingAnimation.h"
#include "AnimationTools.h"

class IdleAnimation : public Animation {
public:
    IdleAnimation(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive, StickGesture* gesture);
    ~IdleAnimation();

    void idleAnimation();
    CRGB* loopStep() override { idleAnimation(); return leds; }
    bool isFinished() const override { return false; }
    bool isInterruptible() const override { return true; }
    void stop() override {}

private:
    void updateMovementState(StickGesture::MovementState newState);

    CRGB* leds;
    CRGB* stillBuffer;
    CRGB* movingBuffer;
    uint16_t numLeds;
    uint8_t ledsAlive;

    IdleStillAnimation* stillAnimation;
    IdleMovingAnimation* movingAnimation;

    StickGesture::MovementState currentMovementState;
    StickGesture::MovementState previousMovementState;
    StickGesture* gesture;

    // Transition state
    bool isTransitioning;
    unsigned long transitionStartTime;
    static constexpr unsigned long TRANSITION_DURATION_MS = 2000;
};

#include "DebugAnimation.h"

#endif // STAFF_IDLEANIMATION_H

