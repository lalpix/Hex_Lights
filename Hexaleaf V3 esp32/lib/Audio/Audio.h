#ifndef __AUDIO_H_
#define __AUDIO_H_
#include <driver/i2s.h>
#include <stdint.h>
#include <stdlib.h>
#include <arduinoFFT.h>
#include <math.h>
#include "structs.h"

// you shousldn't need to change these settings
// most microphones will probably default to left channel but you may need to tie the L/R pin low
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_26
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_19
#define I2S_MIC_SERIAL_DATA GPIO_NUM_21

#define HIST_NUM        3
#define FREQ_BAND_COUNT 20
/* ----- General constants ----- */
#define SAMPLE_RATE     44100
/* ----- FFT constants ----- */


#define FFT_SAMPLE_COUNT        2048
#define FFT_FREQ_BIN_COUNT      FFT_SAMPLE_COUNT / 2
#define FFT_FREQ_STEP           SAMPLE_RATE / FFT_SAMPLE_COUNT

/* ----- FFT variables ----- */
typedef float fftData_t;

static int16_t raw_samples[FFT_SAMPLE_COUNT] = {0};

static double fftDataReal_[FFT_SAMPLE_COUNT] = {0.0};

static double fftDataImag_[FFT_SAMPLE_COUNT] = {0.0};
static fftData_t kFFT_SamplingFreq = (fftData_t)SAMPLE_RATE;

static fftData_t magnitudeSpectrumAvg_[FFT_FREQ_BIN_COUNT] = {0};

static arduinoFFT fft_ = arduinoFFT(fftDataReal_, fftDataImag_, FFT_SAMPLE_COUNT, kFFT_SamplingFreq); // Create FFT object
/* ----- i2s constants ----- */

static i2s_bits_per_sample_t kI2S_BitsPerSample = I2S_BITS_PER_SAMPLE_16BIT;

static uint8_t kI2S_BytesPerSample = kI2S_BitsPerSample / 8;

static uint16_t kI2S_ReadSizeBytes = FFT_SAMPLE_COUNT * kI2S_BytesPerSample ;

static uint16_t kI2S_BufferSizeSamples = 1024;

static uint16_t kI2S_BufferSizeBytes = kI2S_BufferSizeSamples * kI2S_BytesPerSample;

static uint16_t kI2S_BufferCount = (3 * FFT_SAMPLE_COUNT) / (2 * kI2S_BufferSizeSamples);

static uint8_t kI2S_BufferCountPerFFT = FFT_SAMPLE_COUNT / kI2S_BufferSizeSamples;

static int kI2S_QueueLength = 16;

/* ----- i2s variables ----- */

static int16_t micReadBuffer_[FFT_SAMPLE_COUNT] = {0};

static QueueHandle_t pI2S_Queue_ = nullptr;

// Frequency bands
// Source: https://www.teachmeaudio.com/mixing/techniques/audio-spectrum
//
// Sub-bass:       20-60 Hz
// Bass:           60-250 Hz
// Low midrange:   250-500 Hz
// Midrange:       500-2000 Hz
// Upper midrange: 2000-4000 Hz
// Presence:       4000-6000 Hz
// Brilliance:     6000-20000 Hz

// 20Hz, 25Hz, 31.5Hz, 40Hz, 50Hz, 63Hz, 80Hz, 100Hz, 125Hz 160Hz, 200Hz, 250Hz, 315Hz, 400Hz, 500Hz, 630Hz, 800Hz, 1kHz, 1.25kHz, 1.6kHz, 2kHz, 2.5kHz, 3.15kHz, 4kHz, 5kHz, 6.3kHz, 8kHz, 10kHz, 12.5kHz, 16kHz, 20kHz

// const uint8_t FREQ_BAND_COUNT = 7;
// const float kFreqBandStartHz = 20;
// const float kFreqBandEndHz[FREQ_BAND_COUNT] = {60, 250, 500, 2000, 4000, 6000, 20000};

static float kFreqBandStartHz = 20;

// Index:                                         0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18      19
static float kFreqBandEndHz[FREQ_BAND_COUNT] = {30, 50, 75, 100, 140, 180, 225, 270, 350, 440, 550, 700, 900, 1100, 1400, 1800, 2200, 2800, 3550, 18000};

static float kFreqBandAmp[FREQ_BAND_COUNT] = {0.15f, 0.3f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.3f, 0.4f, 0.4f, 0.4f, 0.5f, 0.8f, 1, 1, 1, 1, 1, 0.3f};
static float bandMagHistory[FREQ_BAND_COUNT][HIST_NUM] = {0.0f};

static int hist_ptr = 0;
static fftData_t sensitivityFactor_ = 1;
static float kSensitivityFactorMax = 1000.0f;
static float magnitudeBandMax_[FREQ_BAND_COUNT] = {0.0f};

/* Start indices, end indices and normalization factors for each frequency band
    The indices relate to the frequency bins resulting from the FFT */
static uint16_t freqBandBinIdxStart_[FREQ_BAND_COUNT] = {0};
static uint16_t freqBandBinIdxEnd_[FREQ_BAND_COUNT] = {0};
static uint16_t freqBandBinCount_[FREQ_BAND_COUNT] = {0};

/* ----- Beat detection constants and variables ----- */

static uint8_t kBeatDetectBandOffset = 1;

static float BeatThresholdMultyplier = 1.3f;//4

static float beatHist_[FREQ_BAND_COUNT][HIST_NUM] = {0.0f};

static uint8_t beatVisIntensity_[FREQ_BAND_COUNT] = {0};
bool setupI2Smic();
bool setupSpectrumAnalysis();
void newAudioReading(float *magnitudeBand, float *magnitudeBandWeightedMax);
#endif