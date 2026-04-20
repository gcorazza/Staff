//
// Created by gian on 30.03.26.
//

#include "IdleAnimation.h"
#include "../LEDs.h"

IdleAnimation::IdleAnimation(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive, StickGesture* gesture)
    : leds(leds),
      stillBuffer(new CRGB[numLeds]),
      movingBuffer(new CRGB[numLeds]),
      numLeds(numLeds),
      ledsAlive(ledsAlive),
      stillAnimation(new IdleStillAnimation(stillBuffer, numLeds, ledsAlive)),
      movingAnimation(new IdleMovingAnimation(movingBuffer, numLeds)),
      currentMovementState(StickGesture::MovementState::Moving),
      previousMovementState(StickGesture::MovementState::Moving),
      gesture(gesture),
      isTransitioning(false),
      transitionStartTime(0)
{
    // Initialize buffers to black
    for (uint16_t i = 0; i < numLeds; i++) {
        stillBuffer[i] = CRGB::Black;
        movingBuffer[i] = CRGB::Black;
    }
}

IdleAnimation::~IdleAnimation()
{
    delete stillAnimation;
    delete movingAnimation;
    delete[] stillBuffer;
    delete[] movingBuffer;
}

void IdleAnimation::updateMovementState(StickGesture::MovementState newState)
{
    if (currentMovementState != newState) {
        previousMovementState = currentMovementState;
        currentMovementState = newState;

        // Start transition on state change
        isTransitioning = true;
        transitionStartTime = millis();

        if (previousMovementState == StickGesture::MovementState::Still &&
            currentMovementState == StickGesture::MovementState::Moving)
        {
            Serial.println(F("[IdleAnimation] Transition: Still -> Moving"));
            stillAnimation->beginBurst(millis());
        }

        // Tell the old animation to stop gracefully
        if (currentMovementState == StickGesture::MovementState::Still) {
            movingAnimation->stop();
        } else {
            stillAnimation->stop();
        }
    }
}

void IdleAnimation::idleAnimation()
{
    // Check movement state
    if (gesture) {
        updateMovementState(gesture->getMovementState());
    }

    const unsigned long now = millis();

    // Determine which animation to render based on state
    if (isTransitioning) {
        // During transition: render both animations
        stillAnimation->render(now);
        movingAnimation->render(now);

        // Check if old animation has finished
        Animation* oldAnimation = (currentMovementState == StickGesture::MovementState::Still)
                                   ? static_cast<Animation*>(movingAnimation)
                                   : static_cast<Animation*>(stillAnimation);

        if (oldAnimation->isFinished()) {
            // Transition complete
            isTransitioning = false;

            // Reset the finished animation for next time
            if (currentMovementState == StickGesture::MovementState::Still) {
                delete movingAnimation;
                movingAnimation = new IdleMovingAnimation(movingBuffer, numLeds);
            } else {
                delete stillAnimation;
                stillAnimation = new IdleStillAnimation(stillBuffer, numLeds, ledsAlive);
            }

            // Direct copy without add to save performance
            if (currentMovementState == StickGesture::MovementState::Still) {
                memcpy(leds, stillBuffer, numLeds * sizeof(CRGB));
            } else {
                memcpy(leds, movingBuffer, numLeds * sizeof(CRGB));
            }
        } else {
            // Add both animations together during transition
            AnimationTools::add(stillBuffer, movingBuffer, leds, numLeds);
        }
    } else {
        // No transition, only render active animation to save performance
        if (currentMovementState == StickGesture::MovementState::Still) {
            stillAnimation->render(now);
            memcpy(leds, stillBuffer, numLeds * sizeof(CRGB));
        } else {
            movingAnimation->render(now);
            memcpy(leds, movingBuffer, numLeds * sizeof(CRGB));
        }
    }
}

