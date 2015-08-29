#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pigpio.h>

#define RED     17
#define GREEN   22
#define BLUE    27

static void setColor(const char r, const int g, const int b) {
    gpioPWM(RED, r);
    gpioPWM(GREEN, g);
    gpioPWM(RED, b);
}

static void cycleColor(const useconds_t wait) {
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
        setColor((char)(r * 255), (char)(g * 255), (char)(b * 255));
        usleep(wait);
    }
}


int main(int argc, char *argv[]) {
    if (gpioInitialise() < 0) {
        return EXIT_FAILURE;
    }
    gpioSetMode(RED, PI_OUTPUT);
    gpioSetMode(GREEN, PI_OUTPUT);
    gpioSetMode(BLUE, PI_OUTPUT);
    
    setColor(255, 0, 0);
    
    usleep(500000);
    
    cycleColor(10000);
    
    gpioTerminate();
    
	return EXIT_SUCCESS;
}

