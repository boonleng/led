#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define RED_COLOR              (color_t) {{1.0f}, {0.0f}, {0.0f}}
#define BLUE_COLOR             (color_t) {{0.0f}, {0.0f}, {1.0f}}
#define GREEN_COLOR            (color_t) {{0.0f}, {1.0f}, {0.0f}}
#define YELLOW_COLOR           (color_t) {{1.0f}, {1.0f}, {0.0f}}
#define MAGENTA_COLOR          (color_t) {{0.0f}, {0.0f}, {1.0f}}
#define CYAN_COLOR             (color_t) {{0.0f}, {1.0f}, {1.0f}}
#define WHITE_COLOR            (color_t) {{1.0f}, {1.0f}, {1.0f}}
#define BLACK_COLOR            (color_t) {{0.0f}, {0.0f}, {0.0f}}

#ifndef MIN
#define MIN(x, y)              ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y)              ((x) > (y) ? (x) : (y))
#endif

#ifndef CLAMP
#define CLAMP(x, lo, hi)       MIN(MAX(x, lo), hi)
#endif

typedef struct color {
    union {
        float r;
        float h;
        float s0;
        float red;
        float hue;
    };
    union {
        float g;
        float s;
        float s1;
        float green;
        float saturation;
    };
    union {
        float b;
        float v;
        float l;
        float s2;
        float blue;
        float value;
        float lightness;
    };
    union {
        float w;
        float s3;
        float white;
    };
} color_t;

typedef char colorType;
enum {
    kColorTypeRGB = 0,
    kColorTypeHSV = 1
};

color_t hsv2rgb(const color_t);

#endif
