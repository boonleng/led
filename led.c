#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <pigpio.h>

#include "color.h"
#include "ps.h"

#define RED_PIN                19
#define BLUE_PIN               13
#define GREEN_PIN              26
#define WHITE_PIN              6
#define PWM_RANGE              49
#define PWM_FREQUENCY          600
#define EXP_INVERSE            0.36787944117144f
#define EXP_MINUX_EXP_INVERSE  2.35040238728760f
#define BREATH_LOW_INTENSITY   0.1f

enum {
    ModeNull,
    ModeCycleThrough,
    ModeMellow,
    ModePersistent
};

// Global variables
int mode = ModeCycleThrough;
color_t prevHSVColor;
color_t newHSVColor;
color_t newRGBColor = {
    .white = 1.0f
};

char response[256] = "";

// Function implementations
static void waitWhileActive(const float seconds) {
    const uint32_t count = (uint32_t)(100.0f * seconds);
    for (uint32_t k = 0; k < count && mode; k++) {
        usleep(10000);
    }
}


static void showColorValues(const color_t color, const char *label) {
    printf("%s = %.4f %.4f %.4f\n", label, color.r, color.g, color.b);
}


static void setColorInRGBW(const int r, const int g, const int b, const int w) {
    gpioPWM(RED_PIN, r);
    gpioPWM(BLUE_PIN, b);
    gpioPWM(GREEN_PIN, g);
    gpioPWM(WHITE_PIN, w);
}


static void setColor(const color_t color) {
    return setColorInRGBW((int)(PWM_RANGE * color.red),
                          (int)(PWM_RANGE * color.green),
                          (int)(PWM_RANGE * color.blue),
                          (int)(PWM_RANGE * color.white));
}


static color_t getColor(void) {
    int r = gpioGetPWMdutycycle(RED_PIN);
    int b = gpioGetPWMdutycycle(BLUE_PIN);
    int g = gpioGetPWMdutycycle(GREEN_PIN);
    int w = gpioGetPWMdutycycle(WHITE_PIN);
    return (color_t){{(float)r / PWM_RANGE}, {(float)g / PWM_RANGE}, {(float)b / PWM_RANGE}, {(float)w / PWM_RANGE}};
}


static void cycleColor(const useconds_t wait) {
    float k;
    color_t hsvColor = {{0.0f}, {1.0f}, {1.0f}};
    // Increments of 1.0 / 360.0
    for (k = 0.0f; k < 1.0f && mode < ModePersistent; k += 0.00277777777777f) {
        hsvColor.h = k;
        setColor(hsv2rgb(hsvColor));
        usleep(wait);
    }
}

static color_t breath(const useconds_t wait, const color_t color) {
    int r = 0, g = 0, b = 0, w = 0;
    float f, k;
    const float I = 1.0f - BREATH_LOW_INTENSITY;
    for (k = 0.0f; k < 2.0f * M_PI && mode < ModePersistent; k += 0.004f * M_PI) {
        f = I * (expf(-cosf(k)) - EXP_INVERSE) / EXP_MINUX_EXP_INVERSE + BREATH_LOW_INTENSITY;
        r = (int)(PWM_RANGE * f * color.red);
        g = (int)(PWM_RANGE * f * color.green);
        b = (int)(PWM_RANGE * f * color.blue);
        w = (int)(PWM_RANGE * f * color.white);
        setColorInRGBW(r, g, b, w);
        usleep(wait);
    }
    return (color_t){{r}, {g}, {b}};
}

static void shiftColor(const useconds_t wait, const color_t colorStart, const color_t colorEnd, const colorType type) {
    float k;
    color_t color, tmp;

    for (k = 0.0f; k <= 1.0f; k += 0.01f) {
        tmp.s0 = colorStart.s0 - k * (colorStart.s0 - colorEnd.s0);
        tmp.s1 = colorStart.s1 - k * (colorStart.s1 - colorEnd.s1);
        tmp.s2 = colorStart.s2 - k * (colorStart.s2 - colorEnd.s2);
        tmp.s3 = colorStart.s3 - k * (colorStart.s3 - colorEnd.s3);
        if (type == kColorTypeHSV) {
            color = hsv2rgb(tmp);
        } else {
            color = tmp;
        }
        // printf("  - k = %.4f  HSV: %.2f %.2f %.2f --> RGB: %d %d %d\n", k, tmp.h, tmp.s, tmp.v, r, g, b);
        setColor(color);
        usleep(wait);
    }
}

static void blink(const color_t color, const float period, const uint8_t count) {
    int k;
    for (k = 0; k < count && mode < ModePersistent; k++) {
        setColorInRGBW(0, 0, 0, 0);
        waitWhileActive(period);
        setColor(color);
        waitWhileActive(period);
    }
}


static void catchSignal() {
    printf("\nExit nicely ...\n");
    mode = 0;
}


static int handleCommand(PS_attendant *A) {
    int k;
    char str[8];
    color_t currentRGBColor = getColor();
    printf("Command %s\n", A->cmd);
    switch (A->cmd[0]) {
        case 'a':
            // All
            mode = ModeCycleThrough;
            break;
        case 'm':
            // Mellow mode
            mode = ModeMellow;
            sprintf(response, "Mellow mode activated.\n");
            break;
        case 'b':
            mode = ModePersistent;
            shiftColor(1000, currentRGBColor, (color_t){.b = 1.0f}, kColorTypeRGB);
            break;
        case 'g':
            mode = ModePersistent;
            shiftColor(1000, currentRGBColor, (color_t){.g = 1.0f}, kColorTypeRGB);
            break;
        case 'h':
            sprintf(response, "a - All mode\nm - Mellow mode\nr - Red\nb - Blue\ng - Green\nw - White\nq - Quit\n");
            break;
        case 'r':
            mode = ModePersistent;
            shiftColor(1000, currentRGBColor, (color_t){.r = 1.0f}, kColorTypeRGB);
            break;
        case 'c':
        case 'C':
            mode = ModePersistent;
            k = sscanf(A->cmd, "%s %f %f %f %f", str, &newRGBColor.red, &newRGBColor.green, &newRGBColor.blue, &newRGBColor.w);
            if (k != 5) {
                printf("Incomplete command '%s'.\n", A->cmd);
                break;
            }
            printf("Shifting to new RGBW color %.2f %.2f %.2f %.2f\n", newRGBColor.red, newRGBColor.green, newRGBColor.blue, newRGBColor.white);
            shiftColor(1000, currentRGBColor, newRGBColor, kColorTypeRGB);
            break;
        case 'w':
            mode = ModePersistent;
            if (A->cmd[1] == 'w') {
                // Warm white
                shiftColor(1000, currentRGBColor, (color_t){.r = 0.5f, .g = 1.0f, .w = 1.0f}, kColorTypeRGB);
            } else if (A->cmd[1] == 'c') {
                // Cold white
                shiftColor(1000, currentRGBColor, (color_t){.b = 1.0f, .w = 1.0f}, kColorTypeRGB);
            } else {
                shiftColor(1000, currentRGBColor, (color_t){.w = 1.0f}, kColorTypeRGB);
            }
            break;
        case 'q':
            A->state = PS_STATE_CLOSING;
            break;
        default:
            break;
    }
    return 0;
}

static int handleResponse(PS_attendant *A) {
    if (strlen(response) > 0) {
        PS_send_packets(A, response, strlen(response), NULL);
        response[0] = '\0';
    }
    return 0;
}

#pragma mark - Main

int main(int argc, char *argv[]) {

    int j, k = 0;
    int verbose = 0;

    // Initialize the GPIO library
    if (gpioInitialise() < 0) {
        return EXIT_FAILURE;
    }
    gpioSetPWMrange(6, PWM_RANGE);
    gpioSetMode(6, PI_OUTPUT);
    gpioPWM(6, 1);

    // Prepare output pins
    gpioSetPWMrange(RED_PIN,   PWM_RANGE + 1);
    gpioSetPWMrange(GREEN_PIN, PWM_RANGE + 1);
    gpioSetPWMrange(BLUE_PIN,  PWM_RANGE + 1);
    gpioSetMode(RED_PIN,   PI_OUTPUT);
    gpioSetMode(GREEN_PIN, PI_OUTPUT);
    gpioSetMode(BLUE_PIN,  PI_OUTPUT);
    gpioSetPWMfrequency(RED_PIN,   PWM_FREQUENCY);
    gpioSetPWMfrequency(GREEN_PIN, PWM_FREQUENCY);
    gpioSetPWMfrequency(BLUE_PIN,  PWM_FREQUENCY);

    // Catch some signals for graceful exit
    signal(SIGINT,  catchSignal);
    signal(SIGTERM, catchSignal);
    signal(SIGKILL, catchSignal);
    signal(SIGQUIT, catchSignal);

    // Some color variables
    prevHSVColor.h = 0.0f;    prevHSVColor.s = 1.0f;    prevHSVColor.v = 1.0;
    newHSVColor.h  = 0.3f;     newHSVColor.s = 1.0f;     newHSVColor.v = 1.0;
    newRGBColor  = hsv2rgb(newHSVColor);

    if (argc > 1) {
        float h = atof(argv[1]);
        printf("Start with hue = %.2f\n", h);
        newHSVColor.h = h;
    }

    shiftColor(1000,
               BLACK_COLOR,
               hsv2rgb((color_t){
                   .hue = newHSVColor.h,
                   .saturation = 1.0f,
                   .value = BREATH_LOW_INTENSITY
               }),
               kColorTypeRGB);

    // Keep a copy as "previous"
    prevHSVColor.hue = newHSVColor.hue;

    // Socket port
    PS_server *S = PS_init();
    PS_set_terminate_function_to_builtin(S);
    PS_set_command_function(S, &handleCommand);
    PS_set_stream_function(S, &handleResponse);
    PS_set_name_and_logfile(S, "LED", "led.log");
    S->port = 3000;
    PS_run(S);

    while (mode) {
        if (mode == ModePersistent) {
            waitWhileActive(0.1f);
            continue;
        }
        switch (k) {
            case 0:
                // Cycle through the color wheel quickly
                cycleColor(3000);
                break;
            case 1:
            case 2:
            case 6:
            case 7:
            case 9:
            case 10:
                newHSVColor.hue = fmodf(newHSVColor.hue + 0.1f * (float)(rand()) / RAND_MAX, 1.0f);
                newRGBColor = hsv2rgb(newHSVColor);
                // Change color during the low intensity
                shiftColor(1000,
                           hsv2rgb((color_t){{prevHSVColor.h}, {1.0f}, {BREATH_LOW_INTENSITY}}),
                           hsv2rgb((color_t){{newHSVColor.h}, {1.0f}, {BREATH_LOW_INTENSITY}}),
                           kColorTypeRGB);
                breath(10000, newRGBColor);
                prevHSVColor.hue = newHSVColor.hue;
                break;
            case 3:
                for (j = 0; j < 12 && mode < ModePersistent; j++) {
                    blink(hsv2rgb((color_t){{(float)j / 12.0f}, {1.0f}, {1.0f}}), 0.03f, 5);
                }
                break;
            case 4:
            case 5:
                for (j = 0; j < 3 && mode < ModePersistent; j++) {
                    cycleColor(10000);
                }
                break;
            case 8:
                for (j = 0; j < 3 && mode < ModePersistent; j++) {
                    blink((color_t) {.r = 1.0f}, 0.2f, 4);
                    blink((color_t) {.g = 1.0f}, 0.2f, 4);
                    blink((color_t) {.b = 1.0f}, 0.2f, 4);
                }
                break;
            case 11:
                for (j = 0; j < 3 && mode < ModePersistent; j++) {
                    blink((color_t) {.red = 1.0f}, 0.04f, 5);
                    waitWhileActive(0.4f);
                    blink((color_t) {.blue = 1.0f}, 0.04f, 5);
                    waitWhileActive(0.4f);
                }
                break;
            default:
                waitWhileActive(0.1f);
                break;
        }
        if (verbose) {
            showColorValues(newRGBColor, "newRGBColor");
        }
        if (mode == ModeCycleThrough) {
            k = (k + 1) % 12;
        } else {
            k = 10;
        }
    }

    // Fade to black
    shiftColor(1000, getColor(), BLACK_COLOR, kColorTypeRGB);

    // Conclude the GPIO library
    gpioTerminate();

    return EXIT_SUCCESS;
}

