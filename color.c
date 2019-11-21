#include <color.h>

color_t hsv2rgb(const color_t input) {
    color_t output = {{0.0f}, {0.0f}, {0.0f}, {0.0f}};
    float c = input.v * input.s;
    float x = c * (1.0f - fabs(fmod(input.h * 6.0f, 2.0f) - 1.0));
    // Hue dependent piece-wise function
    if (input.h < 0.16666666666666f) {
        output.r = c; output.g = x; output.b = 0;
    } else if (input.h < 0.33333333333333f) {
        output.r = x; output.g = c; output.b = 0;
    } else if (input.h < 0.50000000000000f) {
        output.r = 0; output.g = c; output.b = x;
    } else if (input.h < 0.66666666666666f) {
        output.r = 0; output.g = x; output.b = c;
    } else if (input.h < 0.83333333333333f) {
        output.r = x; output.g = 0; output.b = c;
    } else {
        output.r = c; output.g = 0; output.b = x;
    }
    // Add some amount to each channel to match lightness
    float m = input.v - c;
    output.r = MIN(output.r + m, 1.0f);
    output.g = MIN(output.g + m, 1.0f);
    output.b = MIN(output.b + m, 1.0f);
    return output;
}
