
#ifndef __HEX_DRAW_
#define __HEX_DRAW_

#include <stdint.h>
#include <stdlib.h>
#include "HEX_node.h"
#include "Audio.h"

static CRGB leds[TOTAL_LEDS];
static uint16_t outer_led_adr[TOTAL_LEDS];
static int outer_led_num;
// p
// first-x(-left/right), second-y(up/down)
static int8_t position[NUM_BOXES][2] = {{0, 0}, {1, 1}, {2, 0}, {3, 1}, {3, 3}};
static uint8_t points_of_contact[NUM_BOXES][6]; // nth bit will tell if nth side is in contact

static FillMode preset1[] = {Fill_by_lines_fLeft, Fill_by_lines_fRight};                      // left-right
static FillMode preset2[] = {Fill_by_lines_fTop, Fill_by_lines_fBottom};                       // top-bottom
static FillMode preset3[] = {Fill_by_lines_meet_in_midle_TB, Fill_by_lines_meet_in_midle_LR}; // middle meeting
static FillMode preset4[] = {Fill_by_rotation_fBottom, Fill_by_rotation_fTop};                // rotation

class Hex_controller
{
private:
    CRGB clr_arr[4];
    uint8_t maxBrightness;
    int16_t rainbow;
    int16_t fade;
    bool fill_done;
    bool change;
    Mode mode;
    FillMode fill_mode;
    int8_t step;
    int8_t HorzCount, VertCount;
    uint16_t step_count;
    uint16_t drawEveryNthMs;
    uint32_t lastDrew;
    Hexnode *nodes[NUM_BOXES];
    uint8_t animation_step;
    uint8_t preset;
    HardwareSerial mySerial;
    

public:
    Hex_controller() : clr_arr{(CRGB::Red), (CRGB::Lime), (CRGB::Pink), (CRGB::DarkKhaki)},
                       fade(0),
                       rainbow(0),
                       maxBrightness(60), // 0--100
                       mode(Stationar),
                       change(true),
                       lastDrew(0),
                       fill_done(true),
                       fill_mode(Fill_by_lines_fTop),
                       drawEveryNthMs(200),
                       mySerial(0)
    {
        step = 1;
        step_count = 0;

        FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, TOTAL_LEDS);
        for (uint8_t i = 0; i < NUM_BOXES; i++)
            nodes[i] = new Hexnode(i);
    }
    // debug func
    void show()
    {
        FastLED.show();
    }
    void init()
    {
        calculate_outer_leds();
        create_outer_path();
        calculate_ledCount_inDir();
        //Audio init
        setupAudioData();
        setupI2Smic();
        setupSpectrumAnalysis();
    }
    void calculate_outer_leds();
    void create_outer_path();
    void calculate_ledCount_inDir();
    // basic fill functions
    void fill_leds_on_Vert_lvl(uint8_t lvl, CRGB clr);
    void fill_leds_on_Hor_lvl(uint8_t lvl, CRGB clr);
    void fill_same_dir_sides(CRGB clr, int direc);
    void fill_all_hex(CRGB clr);
    void fill_one_led_all_hex(CRGB clr, uint8_t n);
    // user input
    void set_pre_anim(uint8_t n);
    void set_speed(uint16_t spd);
    void set_color(CRGB clr, int idx);
    void set_brigtness(uint8_t b);
    void set_serial(HardwareSerial &ser);
    void set_fade(bool f);
    void set_rainbow(int r);
    void pause_play(bool f);
    void change_mode(Mode m);
    void change_fill_mode(FillMode new_fill_mode);
    void printCRGB(CRGB clr);
    void update();
};


#endif