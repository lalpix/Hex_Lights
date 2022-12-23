#include "Audio.h"

#include <Arduino.h>
//#include <Hex_draw.h>
#include <FastLED.h>
#include "structs.h"
#include <stdint.h>
#include <stdlib.h>
#include <arduinoFFT.h>
#include <math.h>


//Hex_controller *hexController;
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

int oldBarHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int bandValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int bandBinSize[] = {2,3,5,7,9,13,18,25,36,50,69,97,135,189,264,368};
void setupLedStrip()
{
    FastLED.addLeds<WS2812B, kPinLedStrip>(ledStrip_, kNumLeds);
    FastLED.clear();
    FastLED.setBrightness(kLedStripBrightness);
    //FastLED.setMaxPowerInVoltsAndMilliamps(5, 450); // Set maximum power consumption to 5 V and 450 mA
    ledStrip_[0].setRGB(150, 0, 0);

    FastLED.show();
}
void setup() {
  Serial.begin(115200);
  delay( 2000 ); // power-up safety delay
  
  /*
  Serial.println("> Setup.");
  hexController = new Hex_controller();
  hexController->set_serial(Serial);
  hexController->init();
  hexController->change_mode(RotationOuter);
  hexController->set_fade(true);
  hexController->update();
  last_ch = millis();
*/
  setupI2Smic();
  setupSpectrumAnalysis();

  setupLedStrip();
  delay( 2000 );

}
int hexcolorToInt(char upper, char lower)
{
  int uVal = (int)upper;
  int lVal = (int)lower;
  uVal = uVal > 64 ? uVal - 55 : uVal - 48;
  uVal = uVal << 4;
  lVal = lVal > 64 ? lVal - 55 : lVal - 48;
  //  Serial.println(uVal+lVal);
  return uVal + lVal;
}  
int16_t raw_samples[kFFT_SampleCount]={0};
/// @brief 
void loop() {
    /*if(last_ch+period>millis()){
      last_ch = millis();
      hexController->change_mode(m);
      m = Mode(m + 1);
      if(m==Mode_num)
        m = Stationar;
    }*/
    /*
    while(1){
    ledStrip_[0].setRGB(150, 0, 0);
    ledStrip_[1].setRGB(0, 150, 0);
    ledStrip_[2].setRGB(0, 0, 150);

    FastLED.show();  
      delay( 2000 );
 
    }*/
   esp_err_t i2sErr = ESP_OK;
  
  size_t i2sBytesRead = 0;

  i2s_read(I2S_NUM_0, raw_samples, sizeof(int16_t) * kFFT_SampleCount, &i2sBytesRead, portMAX_DELAY); // delay on this rougly 28ms
  int max_amp = 0;
  int samples_read = i2sBytesRead / sizeof(int16_t);

   // Check i2s error state after reading
    if (i2sErr)
    {
        log_e("i2s_read failure. ESP error: %s (%x)", esp_err_to_name(i2sErr), i2sErr);
    }

    // Check whether right number of bytes has been read
    if (i2sBytesRead != kI2S_ReadSizeBytes)
    {
        log_w("i2s_read unexpected number of bytes: %d", i2sBytesRead);
    }
 /*
  //Serial.printf("read %d samples\n", samples_read);
  for (int i = 0; i<NUM_BANDS; i++){
    bandValues[i] = 0;
  }
  for (int i = 0; i < samples_read; i++)
  {
    fftDataReal_[i] = raw_samples[i];
    fftDataImag_[i] = 0;
    // Serial.printf("raw sample: %d\n",  raw_samples[i]);    
  }
 
  fft_.DCRemoval();
  fft_.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  fft_.Compute(FFT_FORWARD);
  fft_.ComplexToMagnitude();
   for (int i = 2; i < (kFFT_SampleCount/2); i++){       // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
   
   if (fftDataReal_[i] > NOISE) {                    // Add a crude noise filter

      
    //16 bands, 12kHz top band
      if (i<=2 )           bandValues[0]  += (int)fftDataReal_[i];
      if (i>2   && i<=3  ) bandValues[1]  += (int)fftDataReal_[i];
      if (i>3   && i<=5  ) bandValues[2]  += (int)fftDataReal_[i];
      if (i>5   && i<=7  ) bandValues[3]  += (int)fftDataReal_[i];
      if (i>7   && i<=9  ) bandValues[4]  += (int)fftDataReal_[i];
      if (i>9   && i<=13 ) bandValues[5]  += (int)fftDataReal_[i];
      if (i>13  && i<=18 ) bandValues[6]  += (int)fftDataReal_[i];
      if (i>18  && i<=25 ) bandValues[7]  += (int)fftDataReal_[i];
      if (i>25  && i<=36 ) bandValues[8]  += (int)fftDataReal_[i];
      if (i>36  && i<=50 ) bandValues[9]  += (int)fftDataReal_[i];
      if (i>50  && i<=69 ) bandValues[10] += (int)fftDataReal_[i];
      if (i>69  && i<=97 ) bandValues[11] += (int)fftDataReal_[i];
      if (i>97  && i<=135) bandValues[12] += (int)fftDataReal_[i];
      if (i>135 && i<=189) bandValues[13] += (int)fftDataReal_[i];
      if (i>189 && i<=264) bandValues[14] += (int)fftDataReal_[i];
      if (i>264          ) bandValues[15] += (int)fftDataReal_[i];
    }
  }
  // band values proccessing
  for (int i = 0; i < NUM_BANDS-2; i++)
  {
    bandValues[i] = (bandValues[i] / bandBinSize[i]) / AMPLITUDE ;
    bandValues[i] = ((oldBarHeights[i] * 1) + bandValues[i]) / 2;
    oldBarHeights[i] = bandValues[i];
    Serial.printf("%d:%4d ", i,bandValues[i]);

  }
    Serial.printf("\n");
  */
  int32_t blockSum = raw_samples[0];

    for (int i = 1; i < kFFT_SampleCount; i++)
    {
        blockSum += raw_samples[i];
    }
  int16_t blockAvg = blockSum / kFFT_SampleCount;
  //Serial.printf("block average %d \n", blockAvg);
    /*
    // Increment factor for test signal frequency
    if ( slotNr_ == 0 )
    {
        testSignalFreqFactor_ += 1;

        if ( testSignalFreqFactor_ > kFFT_FreqBinCount )
        {
            testSignalFreqFactor_ = 1;
        }
    }
    */

  for (int i = 0; i < kFFT_SampleCount; i++)
    {
        // Corrected input value: Subtract the block average from each sample in order remove the DC component
        int16_t v = raw_samples[i] - blockAvg;

        // Constant for normalizing int16 input values to floating point range -1.0 to 1.0
        const fftData_t kInt16MaxInv = 1.0f / __INT16_MAX__;

        // Input value in floating point representation
        fftData_t r;

        // Compute input value for FFT
        r = kInt16MaxInv * v;
        //Serial.printf("val for fft %f at idx %d\n",r,i);
        // Generate test signal
        //const float k2Pi = 6.2831853f;
        //const float k2PiSampleCountInv = k2Pi * kFFT_SampleCountInv;
        
        //r = sinf( k2PiSampleCountInv * (testSignalFreqFactor_ * i) );
        
        // Store value in FFT input array
        fftDataReal_[i] = r;
        fftDataImag_[i] = 0.0f;
    }
    
    fft_.Compute(FFT_FORWARD);
    /*
    Serial.printf("computed real vals: \n");
    for (int i = 0; i < kFFT_SampleCount; i++){
      Serial.printf("%f ", fftDataReal_[i]);
    }
 Serial.printf("computed imag vals: \n");
    for (int i = 0; i < kFFT_SampleCount; i++){
      Serial.printf("%f ", fftDataImag_[i]);
    }*/
    
    fftData_t magnitudeSum = 0;
     // Compute magnitude value for each frequency bin, i.e. only first half of the FFT results
    for (uint16_t i = 0; i < kFFT_SampleCount; i++)
    {
        float magValNew = sqrtf(fftDataReal_[i] * fftDataReal_[i] + fftDataImag_[i] * fftDataImag_[i]);
        // Update the averaged spectrum using the current values
        const float w1 = 16.0f / 128.0f;
        const float w2 = 1 - w1;

        // Compute low pass filtered magnitude for each frequency bin
        magnitudeSpectrumAvg_[i] = magValNew * w1 + magnitudeSpectrumAvg_[i] * w2;
        
        // Compute overall sum of all (low pass filtered) frequency bins
        magnitudeSum += magnitudeSpectrumAvg_[i];
    }
    float magnitudeBand[kFreqBandCount] = { 0.0f };

    float magnitudeBandWeightedMax = 0.0f;
    
    for (uint8_t bandIdx = 0; bandIdx < kFreqBandCount; bandIdx++)
    {
        // Interate over all frequency bins assigned to the frequency band
        for (uint16_t binIdx = freqBandBinIdxStart_[bandIdx]; binIdx <= freqBandBinIdxEnd_[bandIdx]; binIdx++)
        {
            // Apply maximum norm to the frequency bins of each frequency band
            if ( magnitudeSpectrumAvg_[binIdx] > magnitudeBand[bandIdx] )
                magnitudeBand[bandIdx] = magnitudeSpectrumAvg_[binIdx];
        }

        float magnitudeBandWeighted = magnitudeBand[bandIdx] * kFreqBandAmp[bandIdx];

        // Compute maximum magnitude value for each frequency band
        if ( magnitudeBand[bandIdx] > magnitudeBandMax_[bandIdx] )
        {
            magnitudeBandMax_[bandIdx] = magnitudeBand[bandIdx];
        }

        // Compute maximum magnitude value across all frequency bands
        if ( magnitudeBandWeighted > magnitudeBandWeightedMax )
        {
            magnitudeBandWeightedMax = magnitudeBandWeighted;
        }
    }
    
    // Update the sensitivity factor
    const float s1 = 8.0f / 1024.0f;
    const float s2 = 1.0f - s1;
    sensitivityFactor_ =  min( (250.0f / magnitudeBandWeightedMax) * s1 + sensitivityFactor_ * s2, kSensitivityFactorMax );


    // ----- Beat detection -----

    // Maintain history of last three magnitude values of the bass band
    beatHist_[0] = beatHist_[1];
    beatHist_[1] = beatHist_[2];
    beatHist_[2] = magnitudeBand[kBeatDetectBand] * kFreqBandAmp[kBeatDetectBand] * sensitivityFactor_;

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

    // hexController->update();

}
