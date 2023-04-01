#ifndef _Helpers_H
#define _Helpers_H

#pragma once
#include <stdlib.h>
#include "FastLED.h"
#include "EEPROM.h"
#define eepromStateNUM 169
#define eepromStateAddr 500
#define modeAddr 700
#define rainbowAddr 710
#define speedAddr 720
#define fadeAddr 730
#define powerAddr 740
#define brightnessAddr 750
#define primaryClrAddr 800
#define secondaryClrAddr 900
#define layoutAddr 1000

void eepromInit();
void nblendU8TowardU8(uint8_t &cur, const uint8_t target, uint8_t amount);

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB fadeTowardColor(CRGB &cur, const CRGB &target, uint8_t amount);
int **parseLayout(std::string str, int numBoxes);
CRGB *parseColorFromText(std::string str);
void printCRGB(CRGB clr);
#endif