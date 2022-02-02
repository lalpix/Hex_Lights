#include <stdint.h>
#include <stdlib.h>
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
#define MAX_VERT_LVL 65+1
#define MAX_HORZ_LVL 35+1
/*TODO-- 
rand hex, fading in and out. and make rand hex with color scheme
make preset speed for all modes
*/


CRGB leds[TOTAL_LEDS];
CRGB vert_leds[MAX_VERT_LVL];
CRGB horz_leds[MAX_HORZ_LVL];
CRGB *outer_leds;
uint16_t outer_led_adr[TOTAL_LEDS];
uint16_t outer_led_num;
//first-x(left/right), second-y(up/down)
int8_t position[NUM_BOXES][2]={{0,0},{1,1},{2,0},{3,1},{3,3},{4,4},{5,3}};
uint8_t points_of_contact[NUM_BOXES][6];//nth bit will tell if nth side is in contact
//fuckin chicken didnt do it all the way, not calculating outer sequnce :() - fixed to my setup 
//uint8_t outer_sequence[][2]={{0,15},{35,50},{70,75},{95,100},{120,135},{155,170},{190,210},{180,185},{175,180},{140,145},{105,120},{80,90},{60,65},{55,60},{20,30}};

FillMode preset1[]={Fill_by_lines_fLeft,Fill_by_lines_fRight}; //left-right
FillMode preset2[]={Fill_by_lines_fTop,Fill_by_lines_fBottom}; //top-bottom
FillMode preset3[]={Fill_by_lines_meet_in_midle_TB,Fill_by_lines_meet_in_midle_LR}; //middle meeting
FillMode preset4[]={Fill_by_rotation_fBottom,Fill_by_rotation_fTop}; //rotation

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
    CRGB colorTo=CRGB::Black;
    Hexnode(uint8_t index)
    {   
      ledStart = index * LEDS_IN_BOX;
      ledEnd = ledStart + LEDS_IN_BOX - 1;
    }
    int side_start_led(uint8_t n){
      return ledStart+((n%6)*LED_IN_SIDE);
    }
    void fill_side(CRGB clr, uint8_t n){//n is <0-5>
      int startOffset = ledStart+(n*LED_IN_SIDE);
      for(int i =0;i<LED_IN_SIDE;i++){
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
      for(int i = ledStart;i<(ledStart+LEDS_IN_BOX);i++){
            leds[i] = clr;
        }
      color=clr;
    }
    void update(uint8_t fract){

    }
    void fill_one_led(CRGB clr,uint8_t n){
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
};

class Hex_controller{
  private:
    CRGB clr_arr[4];
    uint8_t brightness;
    int16_t rainbow;
    bool fade;
    bool fill_done;
    bool change;
    Mode mode;
    FillMode fill_mode;
    int8_t step;
    uint16_t step_count;   
    uint16_t drawEveryNthMs;
    uint32_t lastDrew;
    Hexnode *nodes[NUM_BOXES];
    uint8_t animation_step;
    uint8_t preset;
    HardwareSerial mySerial;
  public:
    Hex_controller():
      clr_arr{(CRGB::Blue),(CRGB::Lime),(CRGB::Pink),(CRGB::DarkKhaki)},
      fade(true),
      rainbow(1),
      brightness(200),
      mode(Stationar),
      change(true),  
      lastDrew(0),
      fill_done(true),
      fill_mode(Fill_by_lines_fTop),
      drawEveryNthMs(100),
      mySerial(NULL)
    { 
      //calculate_outer_leds();
      //create_outer_path();  
      step=1;
      step_count=0;

      FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, TOTAL_LEDS);
      for (uint8_t i = 0; i < NUM_BOXES; i++)
          nodes[i] = new Hexnode(i);
    }
    //debug func
    
    void test(){
      calculate_outer_leds();
      create_outer_path();
    }
    void calculate_outer_leds(){
      int num_of_contact_points=0;
      int direc_arr[6][2]={{-1,-1},{-1,1},{0,2},{1,1},{1,-1},{0,-2}};
      for(int i=0;i<NUM_BOXES;i++){
        mySerial.print("box num: ");
        mySerial.println(i);
        int x=position[i][0];
        int y=position[i][1];
        for(int k=0;k<6;k++){
          int n = (x+direc_arr[k][0]);
          int m = (y+direc_arr[k][1]);
          for(int q=0;q<NUM_BOXES;q++){
            if(position[q][0]==n && position[q][1]==m && q!=i){
              mySerial.print("has contact at side: ");
              mySerial.print(k);
              mySerial.print(" with box num: ");
              mySerial.println(q);
              points_of_contact[i][k]=q+1; //+1 so 0 is no contant point
              num_of_contact_points++;
            }
          }
        }
      }
      
      outer_led_num=TOTAL_LEDS-(num_of_contact_points*LED_IN_SIDE);
      outer_leds = (CRGB*) malloc(sizeof(CRGB)*outer_led_num);
      mySerial.println(outer_led_num);
    }
    void create_outer_path(){//NOT FOR ALL SETUPS  
      uint16_t led_count_1=0;
      uint16_t led_count_2=0;
      uint8_t start_1=0;
      uint8_t end_1 =0;
      uint8_t start_2=0;
      uint8_t end_2 =0;
      for(int i=0;i<NUM_BOXES;i++){
        start_1=nodes[i]->side_start_led(0);
        end_1 =nodes[i]->side_start_led(0) +30;
        start_2=nodes[i]->side_start_led(0);
        end_2 =nodes[i]->side_start_led(0) +30;
        for(int j=0;j<6;j++){
          if(points_of_contact[i][j]!=0){
            if(points_of_contact[i][j]>i){//its contanct with next box
              end_1=nodes[i]->side_start_led(j);
              start_2=nodes[i]->side_start_led(j+1);
            }else{//its contantc with previous box
              end_2=(j!=0) ? nodes[i]->side_start_led(j) : nodes[i]->side_start_led(j)+30;
              start_1=nodes[i]->side_start_led(j+1);
            }
          }  
        }
        
        for(int a=start_1;a<end_1;a++){
          outer_led_adr[led_count_1]=a;
          led_count_1++;
        }
        if(start_2<end_2){
          for(int a=end_2;a>start_2;a--){
            outer_led_adr[(outer_led_num-1)- led_count_2]=a-1;
            led_count_2++;
          }
        }else{
          uint16_t end_22 = end_2;
          uint16_t start_22= start_2-(start_2%30);
          end_2 = start_22+30;
          for(int a=end_22;a>start_22;a--){
            outer_led_adr[(outer_led_num-1)- led_count_2]=a-1;
            led_count_2++;
          }
          for(int a=end_2;a>start_2;a--){
            outer_led_adr[(outer_led_num-1)- led_count_2]=a-1;
            led_count_2++;
          } 
        }
      }
    }
    
    //basic fill functions
    void fill_leds_on_Vert_lvl(uint8_t lvl,CRGB clr){
      for (uint8_t i = 0; i < NUM_BOXES; i++){
        int line_lvl= lvl-(10*position[i][0])-1;
        if(line_lvl>=0 && line_lvl<15)
          nodes[i]->fill_vert_line(clr,line_lvl);        
        }
      }
    void fill_leds_on_Hor_lvl(uint8_t lvl,CRGB clr){
      for (uint8_t i = 0; i < NUM_BOXES; i++){
        int line_lvl= lvl-(6*position[i][1]);
        if(line_lvl>=0 && line_lvl<12)
          nodes[i]->fill_hor_line(clr,line_lvl);        
        }
      }
    void fill_same_dir_sides(CRGB clr,int direc){
      direc = direc % 6;
      for(int i=0;i<NUM_BOXES;i++){//side is <0-5>
        nodes[i]->fill_side(clr,direc);
      }
    }
    void fill_all_hex(CRGB clr){
      mySerial.print("filling hex with clr: ");
      printCRGB(clr);
      for(int i = 0;i<NUM_BOXES;i++){
        nodes[i]->fill_hex(clr);
      }
    }
    void fill_one_led_all_hex(CRGB clr,uint8_t n){
      for(int i=0;i<NUM_BOXES;i++){
        nodes[i]->fill_one_led(clr,n);
      }
    }
    //user input
    void set_pre_anim(uint8_t n){
      mode = PresetAnim;
      fill_done = true;
      preset = n;
      step=0;
      animation_step=0;
      change = true;
    }
    void set_speed(uint16_t spd){
      drawEveryNthMs=spd;
      change = true;
      }
    void set_color(CRGB clr,int idx){
      clr_arr[idx]=clr;
      printCRGB(clr_arr[idx]);
      change = true;
      }
    void set_brigtness(uint8_t b){
      brightness=b;
      change = true;
    }
    void set_serial(HardwareSerial &ser){
      mySerial = ser;
    }
    void set_fade(bool f){
      fade=f;
    }
    void set_rainbow(int r){
      if(r==1){
        rainbow = rgb2hsv_approximate(clr_arr[0]).h;
      }else{rainbow=0;}
      mySerial.print("rainbow is now: ");
      mySerial.println(rainbow);
    }
    void pause_play(bool f){
      change = f ? true : false;
    }
  
    //internal functions
    void my_fill_rainbow(CRGB *myLeds,uint16_t num,uint8_t initial_hue){
      float _step=(float)255/(float)num;//255 is max hue
      int h=0;
      mySerial.print("step is ");
      mySerial.print(_step);
      mySerial.print(" hue is ");
      for(int i = 0;i<num;i++){
        h=((int)(initial_hue+(i*_step)))%255;
        mySerial.print(h);
         mySerial.print(" ");
        myLeds[i].setHue(h);
      }
    }

    void change_mode(int m){
      fill_mode = Not_fill_mode;
      fill_done = true;
      change = true;
      mode = (Mode)m;
      preset = 0;
      step_count=0;
      step=1;
      }
    void change_fill_mode(FillMode new_fill_mode){
      mySerial.print("changing fill mode to: ");
      mySerial.println(new_fill_mode);
      fill_mode = new_fill_mode;
      fill_done = false;
      lastDrew = 0;
      step_count=0;
      step = 1;

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
      mySerial.print("step count is: ");
      mySerial.println(step_count);
      mySerial.print("step is: ");
      mySerial.println(step);
    }
    void printCRGB(CRGB clr){
      mySerial.print("R: ");
      mySerial.print(clr.r,HEX);
      mySerial.print(" G: ");
      mySerial.print(clr.g,HEX);
      mySerial.print(" B: ");
      mySerial.println(clr.blue,HEX);
    }
    
    void update(){
      if(millis() > (lastDrew + drawEveryNthMs) && change){
        if(rainbow>0){
          mySerial.print("rainbow hue is: ");
          mySerial.println(rainbow);
          clr_arr[0].setHue(rainbow);
          printCRGB(clr_arr[0]);
          rainbow++;
          if(rainbow>=255)
            rainbow=1;  
        }
        switch(mode){
          case Stationar:{
            mySerial.println("stationar");
            fill_all_hex(clr_arr[0]);
            //change = false;
            break;  }
          case StationarOuter:{
            for(int i=0;i<NUM_BOXES;i++){
              for(int j=0;j<6;j++){
                if (!(points_of_contact[i][j])){//negated
                nodes[i]->fill_side(clr_arr[0],j);
                mySerial.print("Set side: ");
                mySerial.print(j);
                mySerial.print(" in box ");
                mySerial.println(i);
                }
              }
            }
          break;}
          case Rotation:{
            mySerial.println("rotate");
            for(int i=0;i<NUM_BOXES;i++){
              if(!fade){
                fill_same_dir_sides(clr_arr[0],step_count+1);
                fill_same_dir_sides(CRGB::Black,step_count);
                }
              else{
                fill_one_led_all_hex(clr_arr[0],step_count%LEDS_IN_BOX);
              }
              step_count++;}                 
            break;}
          case RotationOuter:{////this needs testing -----------------------------
            mySerial.println("rotateOuter"); //using preset as util var
            if(step_count>=outer_led_num)
              step_count=0;
            leds[outer_led_adr[step_count]]=clr_arr[0];
            step_count++;           
            break;}
          case RandColorRandHex:{
            if(step_count>drawEveryNthMs){
            uint8_t idx = rand()%NUM_BOXES;
            nodes[idx]->fill_hex(CHSV(random8(),255,255));
            }
            step_count++;
            break;}
          case PresetAnim:{
            if(preset!=0 && fill_done){
              animation_step++;
              if(1<animation_step){ //for presets with len 2
                animation_step =0;  
                } 
              mySerial.print("preset fill anim step:");
              mySerial.print(animation_step);
              mySerial.print(" and preset: ");
              mySerial.println(preset);
              switch(preset){
                case 1:
                  change_fill_mode(preset1[animation_step]);        
                  break;
                case 2:
                  change_fill_mode(preset2[animation_step]);
                  break;
                case 3:
                  change_fill_mode(preset3[animation_step]);
                  break;
                case 4:
                  change_fill_mode(preset4[animation_step]);
                  break;
                default:
                  //notihing
                  break;
              }
            }
            break;}
          case RainbowSwipeVert:{
            fill_rainbow(vert_leds,MAX_VERT_LVL,step_count);
            for(int i=0;i<=MAX_VERT_LVL;i++){
              //CRGB clr=clr.setHue(((step_count+i)*step)%255);
              fill_leds_on_Vert_lvl(i,vert_leds[i]);
            }
            step_count= (step_count>255) ? 0 : step_count+1; 
            break;}
          case RainbowSwipeHorz:{
            fill_rainbow(horz_leds,MAX_HORZ_LVL,step_count);
            for(int i=0;i<MAX_HORZ_LVL;i++){  
              //CRGB clr=clr.setHue(((step_count+i)*step)%255);
              fill_leds_on_Hor_lvl(i,horz_leds[i]);
              }
            step_count= (step_count>255) ? 0 : step_count+1;
            break;}
          case RainbowOuter:{
            my_fill_rainbow(outer_leds,outer_led_num,step_count);
            for(int i=0;i<outer_led_num;i++){
              leds[outer_led_adr[i]]=outer_leds[i];
            }
            step_count= (step_count>255) ? 0 : step_count+1;
            break;}
          default:
            mySerial.println("ERROR: False mode");
          }
        if(!fill_done && fill_mode!= Not_fill_mode){  
          mySerial.print("filling with fillMode: ");
          mySerial.println(fill_mode);
          mySerial.print("Step count is: ");
          mySerial.println(step_count);
          mySerial.print("Step is: ");
          mySerial.println(step);
          if(rainbow){clr_arr[animation_step]=clr_arr[0];}
          switch (fill_mode){
            case Fill_by_lines_fTop :
            case Fill_by_lines_fBottom :
              if(step_count>= 0 && MAX_HORZ_LVL >= step_count){
                fill_leds_on_Hor_lvl(step_count,clr_arr[animation_step]);}
                else{fill_done=true;}
              break;
            case Fill_by_lines_fLeft:
            case Fill_by_lines_fRight:
              if(step_count>=0 && MAX_VERT_LVL >= step_count){ 
                fill_leds_on_Vert_lvl(step_count,clr_arr[animation_step]);}
                else{fill_done=true;}
              break;
            case Fill_by_lines_meet_in_midle_LR:
              if(step_count>= 0 && (MAX_VERT_LVL/2) >= step_count){
                fill_leds_on_Vert_lvl(step_count,clr_arr[animation_step]);
                fill_leds_on_Vert_lvl(MAX_VERT_LVL-step_count,clr_arr[animation_step]);}
                else{fill_done=true;}
              break;
            case Fill_by_lines_meet_in_midle_TB:
              if(step_count>= 0 && (MAX_HORZ_LVL/2) >= step_count){
                fill_leds_on_Hor_lvl(step_count,clr_arr[animation_step]);
                fill_leds_on_Hor_lvl(MAX_HORZ_LVL-step_count,clr_arr[animation_step]);}
                else{fill_done=true;}
              break;
            case Fill_by_rotation_fTop:
            case Fill_by_rotation_fBottom:
              if(step_count <= TOTAL_LEDS){
                leds[step_count]= clr_arr[animation_step];}
              else{fill_done=true;}
              break;
            }
            step_count+=step;
          }
        lastDrew=millis();
        if(fade){
          for(int i=0;i<TOTAL_LEDS;i++){//use brightness
            leds[i].fadeToBlackBy(brightness);
            }
        }
        FastLED.show();
      
    }
  }
};
