// Animation.h
// Abstrakte Basisklasse für Animationen

#ifndef STAFF_ANIMATION_H
#define STAFF_ANIMATION_H

#include <FastLED.h>

class Animation {
public:
    virtual ~Animation() = default;
    virtual CRGB* loopStep() = 0;
    virtual bool isFinished() const = 0;
    virtual bool isInterruptible() const = 0;
    virtual void animationStart() {}
    virtual void stop() = 0;  // Gracefully stop the animation (e.g., don't spawn new elements)
};

#endif // STAFF_ANIMATION_H
