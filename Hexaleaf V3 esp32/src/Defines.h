#ifndef DEFINES
#define DEFINES
#include "FastLED.h"

/* Number of LEDs in each box/leaf */
#define LEDS_IN_BOX 30
/*The number of boxes */
#define NUM_BOXES 2
/*The pin the LED is connected to */
#define LED_PIN 16
/*Don't change unless you know what you're doing */
#define TOTAL_LEDS LEDS_IN_BOX *NUM_BOXES

#define LED_IN_SIDE LEDS_IN_BOX/6

#define BOX_LED_NUM_HEIGHT  2*(LED_IN_SIDE+1) 

#define MAX_VERT_LVL 65+1 //HARDWIRED
#define MAX_HORZ_LVL 35+1
/*TODO-- 
rand hex, fading in and out. and make rand hex with color scheme
make preset speed for all modes
*/
#endif