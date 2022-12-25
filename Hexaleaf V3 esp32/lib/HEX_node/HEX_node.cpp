#include "Hex_node.h"

int Hexnode::side_start_led(uint8_t n){
  return ledStart+((n%6)*LED_IN_SIDE);
}
CRGB*  Hexnode::fill_side(CRGB clr,CRGB* leds, uint8_t n){//n is <0-5>
  int startOffset = ledStart+(n*LED_IN_SIDE);
  for(int i =0;i<LED_IN_SIDE;i++){
      leds[startOffset+i] = clr;
  }
  return leds;
}
CRGB* Hexnode::fill_vert_line(CRGB clr,CRGB* leds,uint8_t line){
  if(line<5){ 
      leds[ledStart+5+line] = clr;
      leds[ledStart+4-line] = clr;    
  }else if(line>=5){
      leds[ledStart+5+line]=clr;
      leds[ledStart+(34-line)]=clr;
  }
  return leds;
}
CRGB* Hexnode::fill_hor_line(CRGB clr,CRGB* leds,uint8_t line){
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
  return leds;
}
CRGB* Hexnode::fill_hex(CRGB clr,CRGB* leds){
  for(int i = ledStart;i<(ledStart+LEDS_IN_BOX);i++){
    leds[i] = clr;
    }
  return leds;
  color=clr;
}
CRGB* Hexnode::fill_one_led(CRGB clr,CRGB* leds,uint8_t n){
  if(n<LEDS_IN_BOX)
    leds[ledStart+n]=color;
  return leds;
}
    /*
    CRGB* Hexnode::ledscompute_fade_vector()
    {
        fadeVector.r = (float)(colorTo.r - color.r) / (float)fadeTimeMs;
        fadeVector.g = (float)(colorTo.g - color.g) / (float)fadeTimeMs;
        fadeVector.b = (float)(colorTo.b - color.b) / (float)fadeTimeMs;
    }
    CRGB* Hexnode::ledscolor_update()
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
    int Hexnode::draw(){
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