//
// Created by gian on 20.04.26.
//

#include "IdleStillAnimation.h"
#include <math.h>

namespace {
constexpr uint8_t IDLE_FADE_AMOUNT = 10;
constexpr uint8_t BURST_FADE_AMOUNT = 80;
constexpr uint8_t MAX_BURST_RANGE = 10;
constexpr uint8_t MIN_BURST_RANGE = 1;
constexpr float IDLE_BRIGHTNESS_SCALE = 0.25f;
constexpr float BURST_GROWTH_PER_MS = 0.003f;
}

IdleStillAnimation::IdleStillAnimation(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive)
    : leds(leds),
      numLeds(numLeds),
      ledsAlive(ledsAlive),
      states(new LedState[numLeds]),
      burstActive(false),
      lastBurstUpdate(0),
      shouldStop(false),
      hasStopped(false)
{
    for (uint16_t i = 0; i < numLeds; i++) {
        states[i].active = false;
        states[i].startTime = 0;
        states[i].duration = 0;
        states[i].currentColor = CRGB::Black;
        states[i].currentBrightness = 0;
        states[i].bursting = false;
        states[i].burstRadius = 0.0f;
        states[i].burstMaxRadius = 0.0f;
    }
}

IdleStillAnimation::~IdleStillAnimation()
{
    delete[] states;
}

CRGB* IdleStillAnimation::loopStep()
{
    render(millis());
    return leds;
}

bool IdleStillAnimation::isFinished() const
{
    return hasStopped;
}

void IdleStillAnimation::animationStart()
{
    shouldStop = false;
    hasStopped = false;
}

void IdleStillAnimation::stop()
{
    shouldStop = true;
}

void IdleStillAnimation::render(unsigned long now)
{
    if (burstActive) {
        runBurstFrame(now);
    } else {
        runIdleFrame(now);
    }

    // Check if we're done after stop() was called
    if (shouldStop && !burstActive) {
        bool anyActive = false;
        for (uint16_t i = 0; i < numLeds; i++) {
            if (states[i].active) {
                anyActive = true;
                break;
            }
        }
        if (!anyActive) {
            hasStopped = true;
        }
    }
}

void IdleStillAnimation::runIdleFrame(unsigned long now)
{
    fadeToBlackBy(leds, numLeds, IDLE_FADE_AMOUNT);

    uint8_t activeCount = 0;
    for (uint16_t i = 0; i < numLeds; i++) {
        if (states[i].active) {
            activeCount++;
        }
    }

    // Don't spawn new LEDs if we're stopping
    if (!shouldStop) {
        while (activeCount < ledsAlive) {
            uint16_t idx = random16(numLeds);
            if (!states[idx].active) {
                startLed(idx, now);
                activeCount++;
            }
        }
    }

    for (uint16_t i = 0; i < numLeds; i++) {
        auto& state = states[i];
        if (!state.active) {
            continue;
        }

        float progress = float(now - state.startTime) / float(state.duration);
        if (progress >= 1.0f) {
            state.active = false;
            continue;
        }

        float brightness = sinf(progress * PI) * IDLE_BRIGHTNESS_SCALE;
        brightness = constrain(brightness, 0.0f, 1.0f);

        CRGB color = blend(
            CRGB::White,
            CRGB(150, 200, 255),
            uint8_t(progress * 255)
        );

        color.nscale8(uint8_t(brightness * 255));

        state.currentColor = color;
        state.currentBrightness = brightnessFromColor(color);
        state.bursting = false;

        leds[i] = color;
    }
}

void IdleStillAnimation::runBurstFrame(unsigned long now)
{
    const float deltaMs = (lastBurstUpdate == 0) ? 0.0f : float(now - lastBurstUpdate);
    lastBurstUpdate = now;

    fadeToBlackBy(leds, numLeds, BURST_FADE_AMOUNT);

    bool anyBursting = false;

    for (uint16_t i = 0; i < numLeds; i++) {
        auto& state = states[i];
        if (!state.bursting) {
            continue;
        }

        anyBursting = true;
        state.burstRadius += deltaMs * BURST_GROWTH_PER_MS;

        const float cappedRadius = fmin(state.burstRadius, state.burstMaxRadius);
        const uint8_t maxRange = static_cast<uint8_t>(state.burstMaxRadius);

        if (maxRange == 0) {
            state.bursting = false;
            continue;
        }

        uint8_t scale = (uint8_t)((uint16_t)255 * (maxRange - cappedRadius) / maxRange);
        if (scale < 25) {
            scale = 25;
        }

        CRGB scaled = state.currentColor;
        scaled.nscale8(scale);

        setLedChecked(static_cast<int>(i) + cappedRadius, scaled);
        if (cappedRadius != 0) {
            setLedChecked(static_cast<int>(i) - cappedRadius, scaled);
        }
        if (state.burstRadius >= state.burstMaxRadius) {
            state.bursting = false;
            state.active = false;
        }
    }

    if (!anyBursting) {
        burstActive = false;
        lastBurstUpdate = 0;
    }
}

void IdleStillAnimation::beginBurst(unsigned long now)
{
    bool hasBurstSeed = false;

    for (uint16_t i = 0; i < numLeds; i++) {
        auto& state = states[i];
        if (!state.active) {
            state.bursting = false;
            continue;
        }

        const uint8_t burstRange = computeBurstRange(state.currentBrightness);
        if (burstRange == 0) {
            state.bursting = false;
            continue;
        }
        state.bursting = true;
        state.burstRadius = 0.0f;
        state.burstMaxRadius = burstRange;
        hasBurstSeed = true;
    }

    burstActive = hasBurstSeed;
    if (burstActive) {
        lastBurstUpdate = now;
    }
}

void IdleStillAnimation::startLed(uint16_t index, unsigned long now)
{
    auto& state = states[index];
    state.active = true;
    state.startTime = now;
    state.duration = random16(2000, 3000);
    state.currentColor = CRGB::Black;
    state.currentBrightness = 0;
    state.bursting = false;
    state.burstRadius = 0.0f;
    state.burstMaxRadius = 0.0f;
}

uint8_t IdleStillAnimation::brightnessFromColor(const CRGB& color) const
{
    return max(max(color.r, color.g), color.b);
}

uint8_t IdleStillAnimation::computeBurstRange(uint8_t brightness) const
{
    if (brightness == 0) {
        return 0;
    }

    const float normalized = float(brightness) / 255.0f;
    const uint8_t range = static_cast<uint8_t>(ceilf(normalized * MAX_BURST_RANGE));
    return (range < MIN_BURST_RANGE) ? MIN_BURST_RANGE : range;
}

void IdleStillAnimation::setLedChecked(float index, const CRGB& color)
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

CRGB IdleStillAnimation::applyBrightness(const CRGB& color, float scale)
{
    scale = constrain(scale, 0.0f, 1.0f);
    return CRGB(
        static_cast<uint8_t>(color.r * scale),
        static_cast<uint8_t>(color.g * scale),
        static_cast<uint8_t>(color.b * scale)
    );
}
