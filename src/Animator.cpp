// Animator.cpp
// Implementierung der Animator-Klasse

#include "Animator.h"
#include "effects/DebugAnimation.h"
#include "effects/IdleAnimation.h"
#include "LEDs.h"
#include "PNGAnimation.h"
#include "src/StickGesture.h"

extern StickGesture stickGesture;

Animator::Animator(CRGB* leds, uint16_t numLeds)
    : leds(leds), numLeds(numLeds) {
    // IdleAnimation wird initial nicht gepusht, sondern über ensureIdle
}

void Animator::loopAnimation() {
    if (stack.empty()) return;
    auto& anim = stack.back();
    anim->loopStep();
    FastLED.show();
    if (anim->isFinished()) {
        popAnimation();
    }
}

void Animator::play(std::shared_ptr<Animation> anim, bool interruptible) {
    if (!stack.empty() && !stack.back()->isInterruptible()) {
        queue.push({anim, interruptible});
        return;
    }
    pushAnimation(anim, interruptible);
}

void Animator::play(const char* filename, bool interruptible) {
    auto anim = std::make_shared<PNGAnimation>(filename, leds, numLeds); // leds/numLeds müssen ggf. global oder per Setter übergeben werden
    play(anim, interruptible);
}

void Animator::playIdle(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive) {
    auto anim = std::make_shared<IdleAnimation>(leds, numLeds, ledsAlive, &stickGesture);
    play(anim, true); // Idle ist immer interruptible
}

void Animator::ensureIdle(CRGB* leds, uint16_t numLeds, uint8_t ledsAlive) {
    if (stack.empty()) {
        stack.push_back(std::make_shared<IdleAnimation>(leds, numLeds, ledsAlive, &stickGesture));
    }
}

void Animator::playDebugAnimation() {
    auto anim = std::make_shared<DebugAnimation>(leds, numLeds);
    play(anim, true); // Debug ist immer interruptible
}

void Animator::pushAnimation(std::shared_ptr<Animation> anim, bool /*interruptible*/) {
    anim->animationStart();
    stack.push_back(anim);
}

void Animator::popAnimation() {
    stack.pop_back();
    if (stack.empty() && !queue.empty()) {
        auto next = queue.front();
        queue.pop();
        stack.push_back(next.first);
    }
}
