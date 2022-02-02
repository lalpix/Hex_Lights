#include <stdint.h>
#include "FastLED.h"
#include "structs.h"
/* Number of LEDs in each box/leaf */
#define LEDS_IN_BOX 30
/*The number of boxes */
#define NUM_BOXES 7
/*The pin the LED is connected to */
#define LED_PIN 4
/*Don't change unless you know what you're doing */
#define TOTAL_LEDS LEDS_IN_BOX *NUM_BOXES

#define LED_IN_SIDE LEDS_IN_BOX/6

#define BOX_LED_NUM_HEIGHT  2*(LED_IN_SIDE+1) 
#define MAX_VERT_LVL 65
#define MAX_HORZ_LVL 35

CRGB leds[TOTAL_LEDS];

uint8_t horizontal_lvls[]={0,1,0,1,3,4,3};
uint8_t vertical_lvls[]={0,1,2,3,3,4,5};

class Hexnode
{ 
  private:
    uint8_t horiz_lvl;
    uint8_t vert_lvl;
    uint16_t ledStart;
    uint16_t ledEnd;
    uint16_t fadeTimeMs;
    CRGB color;
    CRGB colorTo;
    FadeVector fadeVector;
    uint8_t spin_index=0;
  public:
    
    Hexnode(uint8_t index)
    {   
        ledStart = index * LEDS_IN_BOX;
        ledEnd = ledStart + LEDS_IN_BOX - 1;
    }
    void fill_side(CRGB clr, uint8_t n){//n is <0-5>
        int startOffset =ledStart+(n*LED_IN_SIDE);
        for(int i =0;i<6;i++){
            leds[startOffset+i] = clr;
        }
    }
    void fill_vert_line(CRGB clr,uint8_t line){
        if(line<5){ 
            leds[ledStart+5+line] = clr;
            leds[ledStart+4-line] = clr;    
        }else if(line>=5){
            leds[ledStart+5+line]=clr;
            leds[ledStart+(34-line)]=clr;
        }
    }
    void fill_hor_line(CRGB clr,uint8_t line){
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
    void fill_hex(CRGB clr){
      for(int i = ledStart;i<LEDS_IN_BOX;i++){
            leds[i] = clr;
        }
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
};

class Hex_controller{
    private:
        CRGB color;
        CRGB color2;
        bool animation_done;
        uint8_t brightness;
        uint8_t preset_animation;
        int8_t animation_phase;
        int8_t step;
        Mode mode;
        uint8_t fill_mode;
        uint16_t step_count;
        uint16_t drawEveryNthMs;
        uint32_t lastDrew;
        Hexnode *nodes[NUM_BOXES];
    public:   
        Hex_controller():  
                lastDrew(0),
                animation_done(false),
                fill_mode(Fill_by_lines_fTop),
                drawEveryNthMs(100),
                step(1),
                step_count(0),
                color(CRGB(0, 60, 120))
        {
            FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, TOTAL_LEDS);
            for (uint8_t i = 0; i < NUM_BOXES; i++)
                nodes[i] = new Hexnode(i);
        }
        void test_func(int k){
            fill_same_dir_sides(color,k);
            if(k==0){
              fill_same_dir_sides(CRGB::Black,5);
            }else{fill_same_dir_sides(CRGB::Black,k-1);}
            FastLED.show();
        }
        //utility functions
        void fill_leds_on_Vert_lvl(uint8_t lvl,CRGB clr){
          clr = brightness_clr_change(clr);
          for (uint8_t i = 0; i < NUM_BOXES; i++){
            int line_lvl= lvl-(10*vertical_lvls[i])-1;
            if(line_lvl>=0 && line_lvl<15)
              nodes[i]->fill_vert_line(clr,line_lvl);        
            }
          }
        void fill_leds_on_Hor_lvl(uint8_t lvl,CRGB clr){
          clr = brightness_clr_change(clr);
          for (uint8_t i = 0; i < NUM_BOXES; i++){
            int line_lvl= lvl-(6*horizontal_lvls[i]);
            if(line_lvl>=0 && line_lvl<12)
              nodes[i]->fill_hor_line(clr,line_lvl);        
            }
          }
        void fill_same_dir_sides(CRGB clr,int direc){
          clr = brightness_clr_change(clr);
          for(int i=0;i<NUM_BOXES;i++){//side is <0-5>
            nodes[i]->fill_side(clr,direc);
          }
        }
        //mode change/setup funcions---------------------------
        void preset_anim(uint8_t n){
          preset_animation=n;
          mode = Preset_anim;
        }
        void set_speed(uint16_t spd){
          drawEveryNthMs=spd;
          }
        void set_color(CRGB clr){
          color=brightness_clr_change(clr);
          }
        void set_brigtness(uint8_t b){
          brightness = b;
          for(int i=0;i<TOTAL_LEDS;i++){
            leds[i].nscale8(b);
            }
          }
        void clear_all(){
          for (uint8_t i = 0; i < NUM_BOXES; i++){
            nodes[i]->fill_hex(CRGB::Black);
          }
        }
        void change_mode(int m){
          mode = (Mode)m;
          animation_done=false;
          preset_animation=0;
          step_count=0;
          }
        CRGB brightness_clr_change(CRGB in_clr){
          CRGB out_clr = in_clr.nscale8(brightness);
          return out_clr;}

        void change_fill_mode(FillMode new_fill_mode,uint16 movement_time_ms,CRGB clr= CRGB::Orange,CRGB clr2= CRGB::Black){
          fill_mode = new_fill_mode;
          color=clr;
          color2=clr2;
          step_count = 0;
          step = 1;
          lastDrew = 0;
          animation_done=false;
          switch (fill_mode)
          {
            case Fill_by_lines_fTop ://horizontal
              step_count = MAX_HORZ_LVL;
              step = -1;
              break;
            case Fill_by_lines_fRight://vertical
              step_count = MAX_VERT_LVL;
              step=-1;
              break;
            case Fill_by_rotation_fTop:
              step_count = TOTAL_LEDS;
              step = -1;
              break;
            case Fill_by_lines_fLeft:
            case Fill_by_lines_fBottom :
            case Fill_by_rotation_fBottom:
            case Fill_by_lines_meet_in_midle_LR:
            case Fill_by_lines_meet_in_midle_TB:
              break;
            default: //Stationar
               drawEveryNthMs = 1000;
              break;  
          }
        }
        void update(){
          if(millis() > (lastDrew + drawEveryNthMs)){
            switch(mode){
              case Stationar: //Stationar       
                change_fill_mode(Fill_by_lines_fLeft,4000,color);
                break;  
              case Rotation:
                for(int i=0;i<NUM_BOXES;i++){

                }
                break;
              case RandColorRandHex:
                break;
              case Preset_anim:
                if(preset_animation && animation_done){
                  switch(preset_animation){
                    case 1://first preset anim
                      switch(animation_phase){
                        case 0:
                          change_fill_mode(Fill_by_lines_fRight,4000,CRGB::Lime);
                          break;
                        case 1:
                          change_fill_mode(Fill_by_lines_fLeft,4000,CRGB::Red);
                          break;
                        default://set to zero and do the zero state
                          animation_phase=0;
                          break;   
                      }           
                      break;
                    case 2:
                      switch(animation_phase){
                        case 0:
                          change_fill_mode(Fill_by_lines_meet_in_midle_TB,4000,CRGB::Blue);
                          break;
                        case 1:
                          change_fill_mode(Fill_by_lines_meet_in_midle_LR,4000,CRGB::Magenta);
                          break;
                        default://set to zero and do the zero state
                          animation_phase=-1;//so that ++ makes it 0
                          break;   
                      }
                      break;
                    case 3:// from all sides
                      switch(animation_phase){
                        case 0:
                          change_fill_mode(Fill_by_lines_fRight,4000,CRGB::Blue);
                          break;
                        case 1:
                          change_fill_mode(Fill_by_lines_fBottom,4000,CRGB::Orange);
                          break;
                        case 2:
                          change_fill_mode(Fill_by_lines_fLeft,4000,CRGB::Green);
                          break;
                        case 3:
                          change_fill_mode(Fill_by_lines_fTop,4000,CRGB::FairyLight );
                          break;
                        default://set to zero and do the zero state
                          animation_phase=-1;//so that ++ makes it 0
                          break;   
                      }
                      break;
                    case 4://rotation up and rotation down
                      switch(animation_phase){
                        case 0:
                          change_fill_mode(Fill_by_rotation_fTop,4000,CRGB::DeepPink);
                          break;
                        case 1:
                          change_fill_mode(Fill_by_rotation_fBottom,4000,CRGB::DeepSkyBlue);
                          break;
                        default://set to zero and do the zero state
                          animation_phase=-1;
                          break;   
                      }
                      break;
                    case 5://rotation
                      break;
                    default:
                      //notihing
                      break;
                  }
                  animation_phase++;
                }
                break;
              }
              if(!animation_done){
                switch (fill_mode){
                  case Fill_by_lines_fTop :
                  case Fill_by_lines_fBottom :
                    if(step_count>= 0 && MAX_HORZ_LVL >= step_count){
                      fill_leds_on_Hor_lvl(step_count,color);}
                      else{animation_done=true;}
                    break;
                  case Fill_by_lines_fLeft:
                  case Fill_by_lines_fRight:
                    if(step_count>=0 && MAX_VERT_LVL >= step_count){ 
                      fill_leds_on_Vert_lvl(step_count,color);}
                      else{animation_done=true;}
                    break;
                  case Fill_by_lines_meet_in_midle_LR:
                    if(step_count>= 0 && (MAX_VERT_LVL/2) >= step_count){
                      fill_leds_on_Vert_lvl(step_count,color);
                      fill_leds_on_Vert_lvl(MAX_VERT_LVL-step_count,color);}
                      else{animation_done=true;}
                    break;
                  case Fill_by_lines_meet_in_midle_TB:
                    if(step_count>= 0 && (MAX_HORZ_LVL/2) >= step_count){
                      fill_leds_on_Hor_lvl(step_count,color);
                      fill_leds_on_Hor_lvl(MAX_HORZ_LVL-step_count,color);}
                      else{animation_done=true;}
                    break;
                  case Fill_by_rotation_fTop:
                  case Fill_by_rotation_fBottom:
                    if(step_count <= TOTAL_LEDS){
                      leds[step_count]= brightness_clr_change(color);}
                    else{animation_done=true;}
                    break;}
                step_count+=step;
              }
            lastDrew=millis();
            FastLED.show();
          }
        }
      
};
