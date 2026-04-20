//
// Created by gian on 20.04.26.
//

#ifndef STAFF_IDLEMOVINGANIMATION_H
#define STAFF_IDLEMOVINGANIMATION_H

#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include "Animation.h"

class IdleMovingAnimation : public Animation {
public:
    IdleMovingAnimation(CRGB* leds, uint16_t numLeds);
    ~IdleMovingAnimation() = default;

    // Animation interface
    CRGB* loopStep() override;
    bool isFinished() const override;
    bool isInterruptible() const override { return true; }
    void stop() override;

    void render(unsigned long now);

private:
    struct MovingShot {
        float position;
        float speed;
        CRGB color;
        unsigned long startTime;
        bool directionUp;
        bool active;
    };

    void setLedChecked(float index, const CRGB& color);
    CRGB applyBrightness(const CRGB& color, float scale);

    CRGB* leds;
    uint16_t numLeds;
    std::vector<MovingShot> movingShots;
    unsigned long lastShotTime;
    bool shouldStop;
    bool hasStopped;
};

#endif // STAFF_IDLEMOVINGANIMATION_H
