//
// Created by gian on 20.04.26.
//

#ifndef STAFF_IDLEMOVINGANIMATION_H
#define STAFF_IDLEMOVINGANIMATION_H

#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include "Animation.h"
#include "../StickGesture.h"

class IdleMovingAnimation : public Animation {
public:
    IdleMovingAnimation(CRGB* leds, uint16_t numLeds, StickGesture* gesture);
    ~IdleMovingAnimation() = default;

    // Animation interface
    CRGB* loopStep() override;
    bool isFinished() const override;
    bool isInterruptible() const override { return true; }
    void animationStart() override;
    void stop() override;

    void render(unsigned long now);

private:
    struct MovingShot {
        float startPosition;
        float speed;  // Positive = up, Negative = down
        CRGB color;
        unsigned long startTime;
        uint16_t fadeTime;  // Fade duration in milliseconds
        bool active;
    };

    struct Spawner {
        float position;
        float speed;  // Positive = up, Negative = down
        unsigned long lastUpdate;
        unsigned long lastShotTime;
        bool active;
    };

    void setLedChecked(float index, const CRGB& color);
    CRGB applyBrightness(const CRGB& color, float scale);

    CRGB* leds;
    uint16_t numLeds;
    StickGesture* gesture;
    std::vector<MovingShot> movingShots;
    Spawner spawner;
    unsigned long animationStartTime;
    unsigned long lastShotTime;
    bool shouldStop;
    bool hasStopped;
};

#endif // STAFF_IDLEMOVINGANIMATION_H
