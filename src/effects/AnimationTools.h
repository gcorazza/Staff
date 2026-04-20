// AnimationTools.h
// Utility functions for animations

#ifndef STAFF_ANIMATIONTOOLS_H
#define STAFF_ANIMATIONTOOLS_H

#include <FastLED.h>
#include <stdint.h>

class AnimationTools {
public:
    // Blends two CRGB colors
    // blend: 0 = full color1, 255 = full color2
    static CRGB blend(const CRGB& color1, const CRGB& color2, uint8_t blendAmount);

    // Blends two LED arrays and stores the result in the output array
    // blend: 0 = full array1, 255 = full array2
    static void blend(CRGB* array1, CRGB* array2, CRGB* output, uint16_t numLeds, uint8_t blend);

    // Adds two LED arrays together and stores the result in the output array
    // Colors are clamped at 255 per channel
    static void add(CRGB* array1, CRGB* array2, CRGB* output, uint16_t numLeds);
};

#endif // STAFF_ANIMATIONTOOLS_H
