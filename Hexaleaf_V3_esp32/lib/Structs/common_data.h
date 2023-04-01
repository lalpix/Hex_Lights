#include <stdlib.h>

#ifndef _Structs
#define _Structs
enum inputMode
{
    Stationar_, // all hex same color
    RotationOuter_,
    StationarOuter_,
    RandColorRandHex_,
    RandColorRandHexFade_,
        RainbowSwipeVert_,
    RainbowSwipeHorz_,
    AudioBeatReact_, //
    AudioFreqPool_,
    TwoColorFading_,
    //--after that it is animation
    TopBottom_,
    LeftRight_,
    MeetInMidle_,
    DirectionCircle_,
    InputMode_num_,
};
String inputModeName(inputMode m)
{
    switch (m)
    {
    case Stationar_:
        return "Stationar";
    case RotationOuter_:
        return "RotationOuter";
    case StationarOuter_:
        return "StationarOuter";
    case RandColorRandHex_:
        return "RandColorRandHex";
    case RandColorRandHexFade_:
        return "RandColorRandHexFade";
    case RainbowSwipeVert_:
        return "RainbowSwipeVert";
    case RainbowSwipeHorz_:
        return "RainbowSwipeHorz";
    case TwoColorFading_:
        return "TwoColorFading";
    case AudioBeatReact_:
        return "AudioBeatReact";
    case AudioFreqPool_:
        return "AudioFreqPool";
    case TopBottom_:
        return "TopBottom";
    case LeftRight_:
        return "LeftRight";
    case MeetInMidle_:
        return "MeetInMidle";
    case DirectionCircle_:
        return "DirectionCircle";
    default:
        return "noMode";
    }
}

#endif
