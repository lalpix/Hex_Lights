
#ifndef __HEX_NODE_
#define __HEX_NODE_

#include "structs.h"
#include "helpers.h"

class Hexnode
{
private:
  uint8_t horiz_lvl;
  uint8_t vert_lvl;
  uint16_t fadeTimeMs;
  FadeVector fadeVector;
  uint8_t spin_index = 0;

public:
  uint16_t ledStart;
  uint16_t ledEnd;
  uint8_t connectionPoints[6] = {0};
  CRGB color = CRGB::DarkGrey;
  CRGB colorTo = CRGB::DarkGrey;
  Hexnode(uint8_t index)
  {
    ledStart = index * LEDS_IN_BOX;
    ledEnd = ledStart + LEDS_IN_BOX - 1;
  }
  int side_start_led(uint8_t n);
  CRGB *fill_side(CRGB clr, CRGB *leds, uint8_t n);
  CRGB *fill_vert_line(CRGB clr, CRGB *leds, uint8_t line);
  CRGB *fill_hor_line(CRGB clr, CRGB *leds, uint8_t line);
  CRGB *fill_n_hor_lines(CRGB lowClr, CRGB highClr, CRGB *leds, uint8_t max);
  CRGB *fill_hex(CRGB clr, CRGB *leds);
  CRGB *fill_one_led(CRGB clr, CRGB *leds, uint8_t n);
  CRGB *fill_n_leds(CRGB clr, CRGB *leds, uint8_t n);
  CRGB *fadeToColor(CRGB *leds, uint8_t amount);
  void setColor(CRGB clr) { color = clr; };
  void setToColor(CRGB clr) { colorTo = clr; };
  void ledscompute_fade_vector();
  void ledscolor_update();
};
#endif