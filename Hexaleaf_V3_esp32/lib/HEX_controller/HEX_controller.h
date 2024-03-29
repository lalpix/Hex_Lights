
#ifndef __HEX_DRAW_
#define __HEX_DRAW_

#include <stdint.h>
#include <stdlib.h>
#include "HEX_node.h"
#include "Audio.h"

static uint16_t *outer_led_adr;
static int outer_led_num;

static int preset_num = 4;
//----ANIMATION PRESETS
// top-bottom
static int preset1[] = {Fill_by_lines_fTop, Fill_by_lines_fBottom};
// left-right
static int preset2[] = {Fill_by_lines_fLeft, Fill_by_lines_fRight};
// middle meeting
static int preset3[] = {Fill_by_lines_meet_in_midle_TB, Fill_by_lines_meet_in_midle_LR};
// From all sides sequentially
static int preset4[] = {Fill_by_lines_fTop, Fill_by_lines_fLeft, Fill_by_lines_fBottom, Fill_by_lines_fRight};
static int *presets[] = {preset1, preset2, preset3, preset4};
static int preset_lenght[] = {sizeof(preset1) / sizeof(FillMode), sizeof(preset2) / sizeof(FillMode), sizeof(preset3) / sizeof(FillMode), sizeof(preset4) / sizeof(FillMode)};

class Hex_controller
{
private:
    int outer_led_num;
    int NumBoxes;
    int TotalLeds;
    uint16_t *outer_led_adr;
    CRGB clr_arr[4]; //defined colors, first 2 can be changed
    uint8_t maxBrightness, fade;
    int16_t rainbow;
    bool fill_done;
    Mode mode;
    FillMode fill_mode;
    int8_t step;
    int8_t HorzCount, VertCount;
    int8_t HorzMax, HorzMin, VertMax, VertMin;
    int32_t step_count;
    uint16_t drawEveryNthMs;
    uint32_t lastDrew;
    Hexnode **nodes;
    uint8_t animation_step;
    uint8_t preset;
    HardwareSerial mySerial;
    CRGB *leds;
    int **position;               
    uint8_t *points_of_contact[6]; // nth bit will tell if nth side is in contact

public:
    Hex_controller(int numBoxes, int **layout) : 
        clr_arr{(CRGB::Red), (CRGB::Lime), (CRGB::Blue), (CRGB::DarkKhaki)},
        fade(30),
        rainbow(0),
        maxBrightness(UINT8_MAX),
        mode(Stationar),
        lastDrew(0),
        fill_done(true),
        fill_mode(Fill_by_lines_fTop),
        drawEveryNthMs(100),
        mySerial(0),
        NumBoxes(numBoxes),
        TotalLeds(numBoxes * LEDS_IN_BOX),
        step(1),
        step_count(0),
        position(layout)
    {
        leds = (CRGB *)calloc(TotalLeds, sizeof(CRGB));
        outer_led_adr = (uint16_t *)calloc(TotalLeds, sizeof(uint16_t));
        nodes = (Hexnode **)calloc(NumBoxes, sizeof(Hexnode *));
        FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, TotalLeds);
        for (uint8_t i = 0; i < numBoxes; i++)
        {
            nodes[i] = new Hexnode(i);   
        }
        Serial.println("Controller ready");
    }
    // debug func
    void show()
    {
        FastLED.show();
    }
    void init(bool firstTime)
    {
        Serial.println("Controller init");
        calculate_outer_leds();
        Serial.println("outer leds calc done");
        create_outer_path();
        Serial.println("outer path calc done");
        calculate_horz_vert_vals();
        Serial.println("horz/vert leds calc done");
        // Audio init
        Serial.println("audio init start");
        if(firstTime){
            setupAudioData();
            setupI2Smic();
            setupSpectrumAnalysis();
        }
        fill_all_hex(CRGB::Black);
    }
    void unbindMic();
    void next_mode();
    void calculate_outer_leds();
    void create_outer_path();
    void calculate_horz_vert_vals();
    void count_preset_lenghts();
    void change_layout(int lenght, int coords[][2]);
    // basic fill functions
    void fill_leds_on_Vert_lvl(int16_t lvl, CRGB clr);
    void fill_leds_on_Hor_lvl(int16_t lvl, CRGB clr);
    void fill_same_dir_sides(CRGB clr, int direc);
    void fill_all_hex(CRGB clr);
    void fill_one_hex(uint8_t hex, CRGB clr);
    void fill_one_led_all_hex(CRGB clr, uint8_t n);
    void fill_one_side_one_hex(CRGB clr, uint8_t hex, uint8_t dir);
    // user input
    void set_pre_anim(uint8_t n);
    void set_speed(uint16_t spd);
    void set_color(CRGB clr, int idx);
    void set_brightness(uint8_t b);
    void set_serial(HardwareSerial &ser);
    void set_fade(uint8_t f);
    void set_rainbow(int r);
    void pause_play(bool f);
    void change_mode(Mode m);
    void change_fill_mode(FillMode new_fill_mode);
    void update();
};

#endif