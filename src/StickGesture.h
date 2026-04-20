#ifndef STICK_GESTURE_H
#define STICK_GESTURE_H

#include <Arduino.h>

class StickGesture {
public:
    enum class Gesture {
        None,
        HitGround,
		Tap,
		DoubleTap,
    };

    enum class MovementState {
        Moving,
        Still
    };

    enum class TabState {
        Idle,
        FirstTap
    };

    StickGesture();

    Gesture loopGesture();
    MovementState getMovementState() const;
    float getAccelerationMagnitudeG();

private:
    unsigned long lastImpactTime;
    float lastgyroMagnitude;
    unsigned long lastMovementTimestamp;
    MovementState movementState;

    // Double Tap
    TabState tabState;
    unsigned long lastTabTime;

    float getGyroMagnitudeDps() const;
    void updateMovementState(float gyroMagnitude, unsigned long now);
};

#endif