import sys
import time
import pigpio

RED=22
BLUE=27
GREEN=17
MAX_RANGE=255

def set_color(red_intensity, green_intensity, blue_intensity):
    pi.set_PWM_dutycycle(RED, red_intensity);
    pi.set_PWM_dutycycle(GREEN, green_intensity);
    pi.set_PWM_dutycycle(BLUE, blue_intensity);

def color_circle(sleep=0.01):
    # assume, maximum brightness & saturation
    c = 1.0

    for k in range(0, 360):
        h = k / 60.0;
        x = c * (1.0 - abs(h % 2.0 - 1.0));
        #x = 1.0 - abs(h % 2.0 - 1.0)
        if h < 1.0:
            r = c; g = x; b = 0;
        elif h < 2.0:
            r = x; g = c; b = 0;
        elif h < 3.0:
            r = 0; g = c; b = x;
        elif h < 4.0:
            r = 0; g = x; b = c;
        elif h < 5.0:
            r = x; g = 0; b = c;
        else:
            r = c; g = 0; b = x;
        # print 'k=%3.0f  h=%6.3f  c=%6.3f  x=%6.3f  (r=%5.2f g=%5.2f b=%5.2f)' % (k, h, c, x, r, g, b);
        set_color(r * MAX_RANGE, g * MAX_RANGE, b * MAX_RANGE);
        time.sleep(sleep)


# connect to the local Pi
#pi = pigpio.pi(victoria.local, 8888);
pi = pigpio.pi();

# initialize the range to be 0 to 1000
pi.set_PWM_range(RED, MAX_RANGE);
pi.set_PWM_range(GREEN, MAX_RANGE);
pi.set_PWM_range(BLUE, MAX_RANGE);


for k in range(0, MAX_RANGE):
    set_color(k, 0, 0)

for k in range(0, MAX_RANGE):
    color_circle()

for k in range(0, MAX_RANGE):
    set_color(MAX_RANGE - k, 0, 0)

set_color(0, 0, 0)

pi.stop();
