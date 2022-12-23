/*#include <stdint.h>
#include <stdlib.h>
#include <FastLED.h>
#include "structs.h"

#define LEDS_IN_BOX 6
#define NUM_BOXES 2

#define LED_IN_SIDE LEDS_IN_BOX/6
class Hexnode
{ 
  private:
    uint8_t horiz_lvl;
    uint8_t vert_lvl;
    uint16_t ledStart;
    uint16_t ledEnd;
    uint16_t fadeTimeMs;
    CRGB color;
    FadeVector fadeVector;
    uint8_t spin_index=0;
  public:
      CRGB colorTo = CRGB::Black;
      Hexnode(uint8_t index)
      {
         ledStart = index * LEDS_IN_BOX;
          ledEnd = ledStart + LEDS_IN_BOX - 1;
        }
    int side_start_led(uint8_t n){
      return ledStart+((n%6)*LED_IN_SIDE);
    }
    void fill_side(CRGB clr, uint8_t n,CRGB *leds){//n is <0-5>
      int startOffset = ledStart+(n*LED_IN_SIDE);
      for(int i =0;i<LED_IN_SIDE;i++){
          leds[startOffset+i] = clr;
      }
    }
    void fill_vert_line(CRGB clr,uint8_t line,CRGB *leds){
      if(line<5){ 
          leds[ledStart+5+line] = clr;
          leds[ledStart+4-line] = clr;    
      }else if(line>=5){
          leds[ledStart+5+line]=clr;
          leds[ledStart+(34-line)]=clr;
      }
    }
    void fill_hor_line(CRGB clr,uint8_t line,CRGB *leds){
      if(line==0){
          for(int i =0;i<5;i++){
            leds[i+ledStart+(5*LED_IN_SIDE)] = clr;
          }
      }else if(line==11){
          for(int i =0;i<5;i++){
            leds[i+ledStart+(2*LED_IN_SIDE)] = clr;
          }
      }else{
          leds[ledStart+(line-1)]=clr;
          leds[ledEnd-LED_IN_SIDE-(line-1)]=clr;
      }
    }
    void fill_hex(CRGB clr,CRGB *leds){
      for(int i = ledStart;i<(ledStart+LEDS_IN_BOX);i++){
            leds[i] = clr;
        }
      color=clr;
    }
    void fill_one_led(CRGB clr,uint8_t n,CRGB *leds){
      if(n<LEDS_IN_BOX)
        leds[ledStart+n]=color;
    }
    /*
    void compute_fade_vector()
    {
        fadeVector.r = (float)(colorTo.r - color.r) / (float)fadeTimeMs;
        fadeVector.g = (float)(colorTo.g - color.g) / (float)fadeTimeMs;
        fadeVector.b = (float)(colorTo.b - color.b) / (float)fadeTimeMs;
    }
    void color_update()
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
//};