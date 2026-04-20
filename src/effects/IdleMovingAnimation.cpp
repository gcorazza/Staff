//
// Created by gian on 20.04.26.
//

#include "IdleMovingAnimation.h"
#include <algorithm>
#include <math.h>

namespace {
constexpr uint8_t IDLE_FADE_AMOUNT = 10;
constexpr uint16_t MOVING_FADE_TIME = 1000;
constexpr uint8_t MOVING_SHOT_START_B = 64;
constexpr uint8_t MOVING_SHOT_START_A = 68;
constexpr float SPAWNER_ACCEL_SPEED_MULTIPLIER = 50.0f;
constexpr float SPAWNER_DECELERATION = 20.0f;  // LEDs per second
constexpr uint16_t SPAWNER_SHOT_INTERVAL_MS = 100;  // Spawner shoots every 50ms
constexpr uint16_t SPAWNER_SHOT_FADE_TIME = 100;  // Short-lived
constexpr uint16_t NORMAL_SHOT_INTERVAL_MS = 500;
constexpr uint16_t NORMAL_SHOT_DURATION = 1000;  // How long normal shots are spawned
}

IdleMovingAnimation::IdleMovingAnimation(CRGB* leds, uint16_t numLeds, StickGesture* gesture)
    : leds(leds),
      numLeds(numLeds),
      gesture(gesture),
      animationStartTime(0),
      lastShotTime(0),
      shouldStop(false),
      hasStopped(false)
{
    // Initialize spawner
    spawner.position = numLeds / 2;
    spawner.speed = 0.0f;  // Start stationary
    spawner.lastUpdate = 0;
    spawner.lastShotTime = 0;
    spawner.active = true;
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

void IdleMovingAnimation::animationStart()
{
    animationStartTime = millis();
    spawner.lastUpdate = animationStartTime;
    shouldStop = false;
    hasStopped = false;
    movingShots.clear();
}

void IdleMovingAnimation::stop()
{
    shouldStop = true;
}

void IdleMovingAnimation::render(unsigned long now)
{
    for (int i = 0; i < numLeds; i++) {
        leds[i] = CRGB::Black;
    }

    if (!shouldStop
        && now - lastShotTime > NORMAL_SHOT_INTERVAL_MS
        && now - animationStartTime < NORMAL_SHOT_DURATION) {
        // Shot going up from bottom
        MovingShot shot1;
        shot1.startPosition = MOVING_SHOT_START_B;
        shot1.speed = -20;
        shot1.color = CHSV(255/2-10 + random(0, 20), 50, 255);
        shot1.startTime = now;
        shot1.fadeTime = MOVING_FADE_TIME;
        shot1.active = true;
        movingShots.push_back(shot1);

        // Shot going down from top
        MovingShot shot2;
        shot2.startPosition = MOVING_SHOT_START_A;
        shot2.speed = 20;
        shot2.color = CHSV(255/2-10 + random(0, 20), 50, 255);
        shot2.startTime = now;
        shot2.fadeTime = MOVING_FADE_TIME;
        shot2.active = true;
        movingShots.push_back(shot2);

        lastShotTime = now;
    }
    // Update spawner
    if (spawner.active && gesture) {
        float deltaTime = (now - spawner.lastUpdate) / 1000.0f;
        spawner.lastUpdate = now;

        // Get acceleration magnitude from gesture
        if (gesture->getMovementState() == StickGesture::MovementState::Moving) {
            // Use acceleration to boost spawner speed (returns magnitude in G)
            float accelMagnitude = gesture->getAccelerationMagnitudeG();

            // Add acceleration boost (scaled by multiplier)
            spawner.speed += accelMagnitude * SPAWNER_ACCEL_SPEED_MULTIPLIER * deltaTime;
        }

        // Apply deceleration towards zero
        if (spawner.speed > 0) {
            spawner.speed -= SPAWNER_DECELERATION * deltaTime;
            if (spawner.speed < 0) spawner.speed = 0;
        } else if (spawner.speed < 0) {
            spawner.speed += SPAWNER_DECELERATION * deltaTime;
            if (spawner.speed > 0) spawner.speed = 0;
        }

        // Update spawner position
        spawner.position += spawner.speed * deltaTime;

        // Bounce at boundaries
        if (spawner.position >= numLeds) {
            spawner.position = numLeds - 1;
            spawner.speed = -abs(spawner.speed);  // Reverse direction
        } else if (spawner.position <= 0) {
            spawner.position = 0;
            spawner.speed = abs(spawner.speed);  // Reverse direction
        }

        // Spawner shoots fast white-to-blue shots
        if (now - spawner.lastShotTime > SPAWNER_SHOT_INTERVAL_MS && shouldStop == false) {
            for (int dir = -1; dir <= 1; dir += 2) {  // -1 = down, +1 = up
                MovingShot shot;
                shot.startPosition = spawner.position;
                shot.speed = dir * random(40, 80);  // Fast shots in both directions
                shot.color = CRGB(random(200, 255), random(200, 255), 255);  // White to blue
                shot.startTime = now;
                shot.fadeTime = SPAWNER_SHOT_FADE_TIME;  // Short-lived
                shot.active = true;
                movingShots.push_back(shot);
            }
            spawner.lastShotTime = now;
        }

        // Draw spawner
        setLedChecked(spawner.position, CRGB::Red);
    }

    // Animate shots
    for (auto& shot : movingShots) {
        if (!shot.active) continue;

        float elapsed = (now - shot.startTime) / 1000.0f;

        // Calculate current position: start + speed * time (speed can be positive or negative)
        float position = shot.startPosition + shot.speed * elapsed;

        // Fade-out calculation
        float fade = 1.0f - float(now - shot.startTime) / shot.fadeTime;
        if (fade < 0.0f) fade = 0.0f;

        CRGB fadedColor = shot.color;
        fadedColor.nscale8(uint8_t(fade * 255));
        setLedChecked(position, fadedColor);

        // Deactivate if outside bounds or faded out
        if (position < 0 || position >= numLeds || fade <= 0.0f) {
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
