#include <stdlib.h>

#ifndef HELPERS
#define HELPERS
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
    default:
        return "noMode";
    }
}

int **parseLayout(std::string str, int numBoxes)
{

    int **arr = new int *[numBoxes];

    int pos = 0, innerPos = 0, boxId = 0;
    std::string delimiterBox = "|";
    std::string delimiterNum = ",";

    std::string token;
    while ((pos = str.find_first_of(delimiterBox)) != std::string::npos)
    {
        arr[boxId] = new int[2];
        token = str.substr(0, pos);
        innerPos = token.find(delimiterNum);

        const char *tmp0 = token.substr(0, innerPos).c_str();
        arr[boxId][0] = atoi(tmp0);

        token.erase(0, innerPos + delimiterNum.length());

        const char *tmp1 = token.c_str();
        arr[boxId][1] = atoi(tmp1);

        str.erase(0, pos + delimiterBox.length());
        boxId++;
    }

    return arr;
}
CRGB *parseColorFromText(std::string str)
{
    CRGB *clr = (CRGB *)calloc(1, sizeof(CRGB));
    std::string delimiter = ",";
    for (size_t i = 0; i < 3; i++)
    {
        int pos = str.find(delimiter);
        const char *tmp0 = str.substr(0, pos).c_str();
        switch (i)
        {
        case 0:
            clr->r = atoi(tmp0);
        case 1:
            clr->g = atoi(tmp0);
            break;
        case 2:
            clr->b = atoi(tmp0);
            break;
        }
        str.erase(0, pos + delimiter.length());
    }
    return clr;
}
void printCRGB(CRGB clr)
{
    Serial.printf("R: %x G: %x B: %x\n", clr.r, clr.g, clr.b);
}
#endif
