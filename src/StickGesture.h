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

    enum class MovementState {
        Moving,
        Still
    };

    StickGesture();

    Gesture loopGesture();
    MovementState getMovementState() const;

private:
    unsigned long lastImpactTime;
    float lastgyroMagnitude;
    unsigned long lastMovementTimestamp;
    MovementState movementState;

    float getAccelerationMagnitudeG();
    float getGyroMagnitudeDps() const;
    void updateMovementState(float gyroMagnitude, unsigned long now);
};

#endif