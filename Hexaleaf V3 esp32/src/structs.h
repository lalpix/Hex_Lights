#ifndef STRUCT_H
#define STRUCT_H
#include <stdlib.h>

union FadeVector {
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

enum FillMode{
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

enum Mode{
  Stationar,//all hex same color
  Rotation, //all hex rotating
  RotationOuter,
  StationarOuter,
  PresetAnim,
  RandColorRandHex,
  RainbowSwipeVert,
  RainbowSwipeHorz,
  Mode_num
};

#endif
