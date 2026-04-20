// AnimationTools.cpp
// Implementation of animation utility functions

#include "AnimationTools.h"

CRGB AnimationTools::blend(const CRGB& color1, const CRGB& color2, uint8_t blendAmount) {
    // Linear interpolation between color1 and color2
    // blendAmount: 0 = full color1, 255 = full color2
    return CRGB(
        color1.r + ((color2.r - color1.r) * blendAmount) / 255,
        color1.g + ((color2.g - color1.g) * blendAmount) / 255,
        color1.b + ((color2.b - color1.b) * blendAmount) / 255
    );
}

void AnimationTools::blend(CRGB* array1, CRGB* array2, CRGB* output, uint16_t numLeds, uint8_t blendAmount) {
    for (uint16_t i = 0; i < numLeds; i++) {
        output[i] = blend(array1[i], array2[i], blendAmount);
    }
}

void AnimationTools::add(CRGB* array1, CRGB* array2, CRGB* output, uint16_t numLeds) {
    for (uint16_t i = 0; i < numLeds; i++) {
        // Add each channel and clamp at 255
        output[i].r = qadd8(array1[i].r, array2[i].r);
        output[i].g = qadd8(array1[i].g, array2[i].g);
        output[i].b = qadd8(array1[i].b, array2[i].b);
    }
}
