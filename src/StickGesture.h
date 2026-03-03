#ifndef STICK_GESTURE_H
#define STICK_GESTURE_H

#include <Arduino.h>

class StickGesture {
public:
    enum class Gesture {
        None,
        HitGround
        // more later
    };

    StickGesture();

    Gesture loopGesture();

private:
    unsigned long lastImpactTime;

    float getAccelerationMagnitudeG();
};

#endif