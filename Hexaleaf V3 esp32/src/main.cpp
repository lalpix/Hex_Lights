
#include <Arduino.h>
#include <FastLED.h>
#include <stdint.h>
#include <stdlib.h>
#include "structs.h"
#include "HEX_node.h"
#include "HEX_controller.h"
//#include "Audio.h"
/* ----- Fastled constants ----- */

const uint8_t kPinLedStrip = 22; // M5StickC grove port, yellow cable

const uint8_t kNumLeds = 150;

const uint8_t kLedStripBrightness = 150;

Hex_controller *hexController;
String topic = "";
String payload="";
String last_payload="";
int red = 100;
int green = 100; 
int blue = 0;
long t=0;
long last_ch = 0;
int period = 5000;
Mode m = Stationar;
CRGB ledStrip_[kNumLeds];

void setupLedStrip()
{
    FastLED.addLeds<WS2812B, kPinLedStrip>(ledStrip_, kNumLeds);
    FastLED.clear();
    //FastLED.setBrightness(kLedStripBrightness);
    //FastLED.setMaxPowerInVoltsAndMilliamps(5, 450); // Set maximum power consumption to 5 V and 450 mA
    ledStrip_[0].setRGB(150, 0, 0);

    FastLED.show();
}
void setup() {
  Serial.begin(115200);
  delay( 2000 ); // power-up safety delay
  
  
  Serial.println("> Setup.");
  hexController = new Hex_controller();
  hexController->set_serial(Serial);
  hexController->init();
  hexController->change_mode(RotationOuter);
  hexController->set_pre_anim(1);
  hexController->set_rainbow(1);
  hexController->set_fade(true);
  hexController->set_speed(100);
  hexController->update();
  last_ch = millis();
/*
  setupI2Smic();
  setupSpectrumAnalysis();
  setupLedStrip();*/
  delay( 2000 );

}


void loop() {
    //delay( 5000 );

    hexController->update();
    /*
        float magnitudeBand[kFreqBandCount] = { 0.0f };

        float magnitudeBandWeightedMax = 0.0f;

        newAudioReading(magnitudeBand, &magnitudeBandWeightedMax);

        // ----- Beat detection -----

        // Maintain history of last three magnitude values of the bass band
        beatHist_[kBeatDetectBand][0] = beatHist_[kBeatDetectBand][1];
        beatHist_[kBeatDetectBand][1] = beatHist_[kBeatDetectBand][2];
        beatHist_[kBeatDetectBand][2] = magnitudeBand[kBeatDetectBand] * kFreqBandAmp[kBeatDetectBand] * sensitivityFactor_;

        float diff1 = beatHist_[1] - beatHist_[0];
        float diff2 = beatHist_[2] - beatHist_[1];

        // Detect magnitude peak
        if ( ((diff1 >= kBeatThreshold) && (diff2 < 0)) || ((diff1 > 0) && (diff2 <= -kBeatThreshold)) )
        {
            Serial.printf("beat detected");
            beatVisIntensity_ = 250;
        }
        else {
            if ( beatVisIntensity_ >= 25 )
                beatVisIntensity_ -= 25;
        }
     // ----- Update the Led strip -----

        if (kNumLeds <= 2*kFreqBandCount + 4)
        {
            // Show beat detection at the beginning of the strip
            const uint8_t numBassLeds = (kNumLeds - kFreqBandCount) / 2;

            for (int i = 0; i < numBassLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }

            // Show frequency intensities on the remaining Leds
            const uint8_t colorStart = 30;
            const uint8_t colorEnd   = 210;
            const uint8_t colorStep  = (colorEnd - colorStart) / kFreqBandCount;

            for (int k = 0; k < kFreqBandCount; k++)
            {
                uint8_t color = colorStart + k * colorStep;
                uint8_t lightness = min( int(magnitudeBand[k] * kFreqBandAmp[k] * sensitivityFactor_), 255);

                ledStrip_[k+numBassLeds].setHSV(color, 255, lightness);
            }

            // Show beat detection at the beginning of the strip
            for (int i = numBassLeds + kFreqBandCount; i < kNumLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }
        }
        else
        {
            // Show beat detection at the beginning of the strip
            const uint8_t numBassLeds = (kNumLeds - 2 * kFreqBandCount) / 2;

            for (int i = 0; i < numBassLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }

            // Show frequency intensities on the remaining Leds
            const uint8_t colorStart = 30;
            const uint8_t colorEnd   = 210;
            const uint8_t colorStep  = (colorEnd - colorStart) / kFreqBandCount;

            for (int k = 0; k < kFreqBandCount; k++)
            {
                uint8_t color = colorStart + k * colorStep;
                uint8_t lightness = min( int(magnitudeBand[k] * kFreqBandAmp[k] * sensitivityFactor_), 255);

                ledStrip_[numBassLeds + k].setHSV(color, 255, lightness);

                ledStrip_[numBassLeds + 2*kFreqBandCount - k - 1].setHSV(color, 255, lightness);
            }

            // Show beat detection at the beginning of the strip
            for (int i = numBassLeds + 2*kFreqBandCount; i < kNumLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }
        }

        FastLED.show();
        float nf;

        if (fabs(magnitudeBand[1]) < 0.001f)
        {
            nf = 1.0f;
        }
        else
        {
            nf = 1.0f / magnitudeBand[1];
        }
        */
    /*
       Serial.printf("0:%04.2f 1:%04.2f 2:%04.2f 3:%04.2f 4:%04.2f 5:%04.2f 6:%04.2f 7:%04.2f 8:%04.2f 9:%04.2f 10:%04.2f 11:%04.2f 12:%04.2f 13:%04.2f 14:%04.2f 15:%04.2f 16:%04.2f 17:%04.2f 18:%04.2f 19:%04.2f Sum:%05.1f Sens: %04.1f \n",//t: %d
                      magnitudeBand[0] * nf,
                      magnitudeBand[1] * nf,
                      magnitudeBand[2] * nf,
                      magnitudeBand[3] * nf,
                      magnitudeBand[4] * nf,
                      magnitudeBand[5] * nf,
                      magnitudeBand[6] * nf,
                      magnitudeBand[7] * nf,
                      magnitudeBand[8] * nf,
                      magnitudeBand[9] * nf,
                      magnitudeBand[10] * nf,
                      magnitudeBand[11] * nf,
                      magnitudeBand[12] * nf,
                      magnitudeBand[13] * nf,
                      magnitudeBand[14] * nf,
                      magnitudeBand[15] * nf,
                      magnitudeBand[16] * nf,
                      magnitudeBand[17] * nf,
                      magnitudeBand[18] * nf,
                      magnitudeBand[19] * nf,
                      0,//magnitudeSum,
                      0//sensitivityFactor_
                      );*/
    // timeDeltaMicros);
}
