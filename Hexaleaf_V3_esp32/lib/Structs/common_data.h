
enum inputMode
{
    Stationar_, // all hex same color
    RotationOuter_,
    StationarOuter_,
    RandColorRandHex_,
    RainbowSwipeVert_,
    RainbowSwipeHorz_,
    AudioBeatReact_, //
    AudioFreqPool_,  //
    TopBottom_,
    LeftRight_,
    MeetInMidle_,
    DirectionCircle_,
};
char *inputModeName(inputMode m)
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
    case RainbowSwipeVert_:
        return "RainbowSwipeVert";
    case RainbowSwipeHorz_:
        return "RainbowSwipeHorz";
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
    }
}
