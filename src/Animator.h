// Animator.h
// Verwalten von Animationen als Stack

#ifndef STAFF_ANIMATOR_H
#define STAFF_ANIMATOR_H

#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <queue>
#include <memory>
#include "effects/Animation.h"

class Animator {
public:
    Animator(CRGB* leds, uint16_t numLeds);
    void loopAnimation();
    void play(std::shared_ptr<Animation> anim, bool interruptible);
    void play(const char* filename, bool interruptible); // PNG
    void playIdle(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive);
    bool isIdleActive() const;

private:
    CRGB* leds;
    uint16_t numLeds;
    std::vector<std::shared_ptr<Animation>> stack;
    std::queue<std::pair<std::shared_ptr<Animation>, bool>> queue;
    void pushAnimation(std::shared_ptr<Animation> anim, bool interruptible);
    void popAnimation();
    void ensureIdle(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive);
};

extern Animator animator;

#endif // STAFF_ANIMATOR_H

