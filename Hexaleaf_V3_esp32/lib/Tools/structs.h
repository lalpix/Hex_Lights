#ifndef STRUCT_H
#define STRUCT_H

/* Number of LEDs in each box/leaf */
#define LEDS_IN_BOX 30
/*The pin the LED is connected to */
#define LED_PIN 22
/*Don't change unless you know what you're doing */

#define LED_IN_SIDE LEDS_IN_BOX / 6

#define BOX_LED_NUM_HEIGHT 2 * (LED_IN_SIDE + 1)

union FadeVector
{
    struct
    {
        float r;
        float g;
        float b;
    };
    struct
    {
        float h;
        float s;
        float v;
    };
};

enum FillMode
{
    Not_fill_mode,
    Fill_by_lines_fTop,
    Fill_by_lines_fBottom,
    Fill_by_lines_meet_in_midle_TB,
    Fill_by_lines_meet_in_midle_LR,
    Fill_by_lines_fLeft,
    Fill_by_lines_fRight,
    Fill_by_rotation_fBottom,
    Fill_by_rotation_fTop,
    Fill_mode_num
};

enum Mode
{

    Stationar, // all hex same color
    RotationOuter,
    StationarOuter,
    RandColorRandHex,
    RandColorRandHexFade,
    RainbowSwipeVert,
    RainbowSwipeHorz,
    AudioBeatReact,
    AudioFreqPool,
    TwoColorFading,
    PresetAnim,
    NotUpdating,
    Mode_num
};

#endif
