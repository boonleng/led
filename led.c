#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <pigpio.h>

#define RED                    22
#define GREEN                  17
#define BLUE                   27
#define PWM_RANGE              200
#define PWM_FREQUENCY          800
#define EXP_INVERSE            0.36787944117144f
#define EXP_MINUX_EXP_INVERSE  2.35040238728760f

int led_run = 1;

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
        float s2;
        float blue;
        float value;
    };
} color_t;

typedef char colorType;
enum {
    kColorTypeRGB = 0,
    kColorTypeHSV = 1
};


void showColorValues(const color_t color, const char *label) {
    printf("%s = %.4f %.4f %.4f\n", label, color.r, color.g, color.b);
}


void setColorInRGB(const int r, const int g, const int b) {
    gpioPWM(GREEN, g);
    gpioPWM(RED, r);
    gpioPWM(BLUE, b);
}


void setColor(const color_t color) {
    return setColorInRGB((int)(PWM_RANGE * color.red),
                         (int)(PWM_RANGE * color.green),
                         (int)(PWM_RANGE * color.blue));
}


color_t hsv2rgb(const color_t input) {
    color_t output = {{0.0f}, {0.0f}, {0.0f}};
    float c = input.v;
    float x = c * (1.0f - fabs(fmod(input.h * 6.0f, 2.0f) - 1.0));
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
    return output;
}

void cycleColor(const useconds_t wait) {
    int k;
    float h, x, r, g, b;
    const float c = 1.0f;
    
    for (k=0; k<360; k++) {
        h = (float)k / 60.0f;
        x = 1.0f - fabs(fmod(h, 2.0f) - 1.0);
        if (h < 1.0f) {
            r = c; g = x; b = 0;
        } else if (h < 2.0f) {
            r = x; g = c; b = 0;
        } else if (h < 3.0f) {
            r = 0; g = c; b = x;
        } else if (h < 4.0f) {
            r = 0; g = x; b = c;
        } else if (h < 5.0f) {
            r = x; g = 0; b = c;
        } else {
            r = c; g = 0; b = x;
        }
        // printf("k=%3d  h=%6.3f\n", k, h);
        setColorInRGB((int)(r * PWM_RANGE), (int)(g * PWM_RANGE), (int)(b * PWM_RANGE));
        if (!led_run) {
            break;
        }
        usleep(wait);
    }
}

void breath(const useconds_t wait, const color_t color) {
    int r, g, b;
    float f, k;
    for (k = 0.0f; k < 2.0f * M_PI; k += 0.004f * M_PI) {
        f = 0.9f * (expf(cosf(k)) - EXP_INVERSE) / EXP_MINUX_EXP_INVERSE + 0.1f;
        r = (int)(PWM_RANGE * f * color.red);
        g = (int)(PWM_RANGE * f * color.green);
        b = (int)(PWM_RANGE * f * color.blue);
        setColorInRGB(r, g, b);
        if (!led_run) {
            break;
        }
        usleep(wait);
    }
}

void shiftColor(const useconds_t wait, const color_t colorStart, const color_t colorEnd, const colorType type) {
    float k;
    color_t color, tmp;
    for (k = 0.0f; k <= 1.0f; k += 0.002f) {
        tmp.s0 = colorStart.s0 - k * (colorStart.s0 - colorEnd.s0);
        tmp.s1 = colorStart.s1 - k * (colorStart.s1 - colorEnd.s1);
        tmp.s2 = colorStart.s2 - k * (colorStart.s2 - colorEnd.s2);
        if (type == kColorTypeHSV) {
            color = hsv2rgb(tmp);
        } else {
            color = tmp;
        }
        // printf("  - k = %.4f  HSV: %.2f %.2f %.2f --> RGB: %d %d %d\n", k, tmp.h, tmp.s, tmp.v, r, g, b);
        setColor(color);
        if (!led_run) {
            break;
        }
        usleep(wait);
    }
}


void catchSignal() {
    printf("Exit nicely ...\n");
    setColorInRGB(0, 0, 0);
    led_run = 0;
}


int main(int argc, char *argv[]) {
    if (gpioInitialise() < 0) {
        return EXIT_FAILURE;
    }
    // Prepare output pins
    gpioSetPWMrange(RED,   PWM_RANGE);
    gpioSetPWMrange(GREEN, PWM_RANGE);
    gpioSetPWMrange(BLUE,  PWM_RANGE);
    gpioSetMode(RED,   PI_OUTPUT);
    gpioSetMode(GREEN, PI_OUTPUT);
    gpioSetMode(BLUE,  PI_OUTPUT);
    gpioSetPWMfrequency(RED,   PWM_FREQUENCY);
    gpioSetPWMfrequency(GREEN, PWM_FREQUENCY);
    gpioSetPWMfrequency(BLUE,  PWM_FREQUENCY);

    // Catch some signals for graceful exit
    signal(SIGINT,  catchSignal);
    signal(SIGTERM, catchSignal);
    signal(SIGKILL, catchSignal);
    signal(SIGQUIT, catchSignal);
    
    setColorInRGB(PWM_RANGE, 0, 0);
    
    usleep(200000);
    
    setColorInRGB(0, PWM_RANGE, 0);
    
    usleep(200000);

    setColorInRGB(0, 0, PWM_RANGE);
    
    usleep(200000);

    color_t prevHSVColor, newHSVColor, newRGBColor;

    prevHSVColor.h = 0.0f;
    prevHSVColor.s = 1.0f;
    prevHSVColor.v = 1.0f;
    
    newHSVColor.h = 0.40f;
    newHSVColor.s = 1.0f;
    newHSVColor.v = 1.0f;
    
    while (led_run) {
        //cycleColor(10000);
        //breath(10000, 0.6f, 1.0f, 1.0f);
        //breath(10000, 0.65f, 1.0f, 0.0f);
        //breath(10000, 0.2f, 1.0f, 1.0f);
        //newHSVColor.hue = (float)(rand() % 1000) * 0.001f;
        
        //shiftColor(5000, prevHSVColor, newHSVColor, kColorTypeHSV);
        newRGBColor = hsv2rgb(newHSVColor);
        //printf("hue = %.4f -- > R:%.4f  G:%.4f  B:%.4f\n", newHSVColor.h, newRGBColor.r, newRGBColor.g, newRGBColor.blue);
        breath(10000, newRGBColor);

        prevHSVColor.hue = newHSVColor.hue;
    }
    
    gpioTerminate();
    
	return EXIT_SUCCESS;
}

