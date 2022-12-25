
#ifndef __HEX_NODE_
#define __HEX_NODE_

#include "FastLED.h"
#include "structs.h"

class Hexnode
{ 
  private:
    uint8_t horiz_lvl;
    uint8_t vert_lvl;
   
    uint16_t fadeTimeMs;
    CRGB color;
    FadeVector fadeVector;
    uint8_t spin_index=0;
  public:
    uint16_t ledStart;
    uint16_t ledEnd;
    uint8_t connectionPoints[6]={0};
    CRGB colorTo=CRGB::Black;
    Hexnode(uint8_t index)
    {   
      ledStart = index * LEDS_IN_BOX;
      ledEnd = ledStart + LEDS_IN_BOX - 1;
    }
    int side_start_led(uint8_t n);
    CRGB *fill_side(CRGB clr, CRGB *leds, uint8_t n);
    CRGB *fill_vert_line(CRGB clr, CRGB *leds, uint8_t line);
    CRGB *fill_hor_line(CRGB clr, CRGB *leds, uint8_t line);
    CRGB *fill_hex(CRGB clr, CRGB *leds);
    CRGB *fill_one_led(CRGB clr, CRGB *leds, uint8_t n);
    /*
    CRGB* ledscompute_fade_vector()
    {
        fadeVector.r = (float)(colorTo.r - color.r) / (float)fadeTimeMs;
        fadeVector.g = (float)(colorTo.g - color.g) / (float)fadeTimeMs;
        fadeVector.b = (float)(colorTo.b - color.b) / (float)fadeTimeMs;
    }
    CRGB* ledscolor_update()
    {
        unsigned long delta_ms = millis() - lastDrawTimer;
        lastDrawTimer = millis();
        int16_t r = color.r + (fadeVector.r * delta_ms);
        int16_t g = color.g + (fadeVector.g * delta_ms);
        int16_t b = color.b + (fadeVector.b * delta_ms);
        (r >= 255) ? color.r = 255 : (r <= 0) ? color.r = 0 : color.r = r;
        (g >= 255) ? color.g = 255 : (g <= 0) ? color.g = 0 : color.g = g;
        (b >= 255) ? color.b = 255 : (b <= 0) ? color.b = 0 : color.b = b;
    }
    int draw(){
      if (animating)
        {
            color_update();
            if (millis() - startDrawTimer >= fadeTimeMs)
            {
                animating = false;
                peakTimer = millis();
            }
        }

      for (uint16_t ledPos = ledStart; ledPos <= ledEnd; ledPos++)
      {
          leds[ledPos] = color;
      }
      return 0;
    }*/ 
};
#endif