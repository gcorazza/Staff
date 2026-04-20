//
// Created by gian on 20.04.26.
//

#include "IdleMovingAnimation.h"
#include <algorithm>
#include <math.h>

namespace {
constexpr uint8_t IDLE_FADE_AMOUNT = 10;
constexpr uint8_t MOVING_SHOOT_AMOUNT = 4;
constexpr uint16_t MOVING_FADE_TIME = 2000;
constexpr uint8_t MOVING_SHOT_START_B = 64;
constexpr uint8_t MOVING_SHOT_OFFSET_FROM_END = 50;
}

IdleMovingAnimation::IdleMovingAnimation(CRGB* leds, uint16_t numLeds)
    : leds(leds),
      numLeds(numLeds),
      lastShotTime(0),
      shouldStop(false),
      hasStopped(false)
{
}

CRGB* IdleMovingAnimation::loopStep()
{
    render(millis());
    return leds;
}

bool IdleMovingAnimation::isFinished() const
{
    return hasStopped;
}

void IdleMovingAnimation::stop()
{
    shouldStop = true;
}

void IdleMovingAnimation::render(unsigned long now)
{
    fadeToBlackBy(leds, numLeds, IDLE_FADE_AMOUNT);

    const float MOVING_SHOT_START_A = numLeds - MOVING_SHOT_OFFSET_FROM_END;

    // Create shots only if not stopping
    if (!shouldStop && now - lastShotTime > 1000 / MOVING_SHOOT_AMOUNT) {
        for (int dir = 0; dir < 2; ++dir) {
            MovingShot shot;
            shot.position = (dir == 0) ? MOVING_SHOT_START_A : MOVING_SHOT_START_B;
            shot.speed = 20; //random(60, 200) / 10.0f; // 6.0 to 20.0 LEDs/second
            shot.color = CHSV(255/2-10 + random(0, 20), 50, 255);
            shot.startTime = now;
            shot.directionUp = (dir == 0);
            shot.active = true;
            movingShots.push_back(shot);
        }
        lastShotTime = now;
    }

    // Animate shots
    for (auto& shot : movingShots) {
        if (!shot.active) continue;
        float elapsed = (now - shot.startTime) / 1000.0f;
        float distance = shot.speed * elapsed;
        // Apply direction:
        shot.position = shot.directionUp ? (MOVING_SHOT_START_A + distance) : (MOVING_SHOT_START_B - distance);

        // Fade-out calculation
        float fade = 1.0f - float(now - shot.startTime) / MOVING_FADE_TIME;
        if (fade < 0.0f) fade = 0.0f;

        CRGB fadedColor = shot.color;
        fadedColor.nscale8(uint8_t(fade * 255));
        setLedChecked(shot.position, fadedColor);

        // Deactivate if outside bounds or faded out
        if (shot.position < 0 || shot.position >= numLeds || fade <= 0.0f) {
            shot.active = false;
        }
    }

    // Remove inactive shots
    movingShots.erase(
        std::remove_if(movingShots.begin(), movingShots.end(), [](const MovingShot& s) { return !s.active; }),
        movingShots.end()
    );

    // Check if finished
    if (shouldStop && movingShots.empty()) {
        hasStopped = true;
    }
}

void IdleMovingAnimation::setLedChecked(float index, const CRGB& color)
{
    if (index < 0 || index >= numLeds) {
        return;
    }

    const int low = static_cast<int>(floorf(index));
    int high = low + 1;
    float percent = index - static_cast<float>(low);
    percent = constrain(percent, 0.0f, 1.0f);

    leds[low] = applyBrightness(color, 1.0f - percent);

    if (high >= numLeds) {
        return;
    }

    leds[high] = applyBrightness(color, percent);
}

CRGB IdleMovingAnimation::applyBrightness(const CRGB& color, float scale)
{
    scale = constrain(scale, 0.0f, 1.0f);
    return CRGB(
        static_cast<uint8_t>(color.r * scale),
        static_cast<uint8_t>(color.g * scale),
        static_cast<uint8_t>(color.b * scale)
    );
}
