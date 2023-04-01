#include "helpers.h"
void eepromInit()
{
    EEPROM.begin(2048);
    if (EEPROM.read(eepromStateAddr) != eepromStateNUM)
    {
        EEPROM.write(eepromStateAddr, eepromStateNUM);
        EEPROM.write(modeAddr, 1);
        EEPROM.write(rainbowAddr, 1);
        EEPROM.write(speedAddr, 1);
        EEPROM.write(fadeAddr, 1);
        EEPROM.write(powerAddr, 1);
        EEPROM.writeString(primaryClrAddr, "0,0,255");
        EEPROM.writeString(secondaryClrAddr, "0,255,0");
        EEPROM.writeString(layoutAddr, "1::0,0|");
        EEPROM.commit();
        Serial.println("eeprom initfill done");
    }
}

void nblendU8TowardU8(uint8_t &cur, const uint8_t target, uint8_t amount)
{
    if (cur == target)
        return;

    if (cur < target)
    {
        uint8_t delta = target - cur;
        delta = scale8_video(delta, amount);
        cur += delta;
    }
    else
    {
        uint8_t delta = cur - target;
        delta = scale8_video(delta, amount);
        cur -= delta;
    }
}

CRGB fadeTowardColor(CRGB &cur, const CRGB &target, uint8_t amount)
{
    nblendU8TowardU8(cur.red, target.red, amount);
    nblendU8TowardU8(cur.green, target.green, amount);
    nblendU8TowardU8(cur.blue, target.blue, amount);
    return cur;
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
