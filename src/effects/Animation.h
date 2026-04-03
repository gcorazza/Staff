// Animation.h
// Abstrakte Basisklasse für Animationen

#ifndef STAFF_ANIMATION_H
#define STAFF_ANIMATION_H

class Animation {
public:
    virtual ~Animation() = default;
    virtual void loopStep() = 0;
    virtual bool isFinished() const = 0;
    virtual bool isInterruptible() const = 0;
};

#endif // STAFF_ANIMATION_H

