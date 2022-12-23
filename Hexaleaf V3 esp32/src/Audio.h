#include <driver/i2s.h>
#include <stdint.h>
#include <stdlib.h>
#include <arduinoFFT.h>
// you shousldn't need to change these settings
// most microphones will probably default to left channel but you may need to tie the L/R pin low
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_26
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_19
#define I2S_MIC_SERIAL_DATA GPIO_NUM_21
#define NOISE           3000
#define NUM_BANDS       16 
#define AMPLITUDE       1000          // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.

/* ----- General constants ----- */

const uint16_t kSampleRate = 44100; // Unit: Hz

/* ----- FFT constants ----- */

typedef float fftData_t;

const uint8_t kFFT_SampleCountLog2 = 11;

const uint16_t kFFT_SampleCount = 1 << kFFT_SampleCountLog2;

const fftData_t kFFT_SampleCountInv = 1.0f / kFFT_SampleCount;

const fftData_t kFFT_SamplingFreq = (fftData_t) kSampleRate;

const uint16_t kFFT_FreqBinCount = kFFT_SampleCount / 2;

const float kFFT_FreqStep = kFFT_SamplingFreq / kFFT_SampleCount;

/* ----- FFT variables ----- */

double fftDataReal_[kFFT_SampleCount] = {0.0};

double fftDataImag_[kFFT_SampleCount] = {0.0};

fftData_t magnitudeSpectrumAvg_[kFFT_FreqBinCount] = {0};

arduinoFFT fft_ = arduinoFFT(fftDataReal_, fftDataImag_, kFFT_SampleCount, kFFT_SamplingFreq); // Create FFT object


/* ----- i2s constants ----- */

const i2s_bits_per_sample_t kI2S_BitsPerSample = I2S_BITS_PER_SAMPLE_16BIT;

const uint8_t kI2S_BytesPerSample = kI2S_BitsPerSample / 8;

const uint16_t kI2S_ReadSizeBytes = kFFT_SampleCount * kI2S_BytesPerSample;

const uint16_t kI2S_BufferSizeSamples = 1024;

const uint16_t kI2S_BufferSizeBytes = kI2S_BufferSizeSamples * kI2S_BytesPerSample;

const uint16_t kI2S_BufferCount = (3 * kFFT_SampleCount) / (2 * kI2S_BufferSizeSamples);

const uint8_t kI2S_BufferCountPerFFT = kFFT_SampleCount / kI2S_BufferSizeSamples;

const int kI2S_QueueLength = 16;

/* ----- i2s variables ----- */

int16_t micReadBuffer_[kFFT_SampleCount] = {0};

QueueHandle_t pI2S_Queue_ = nullptr;

/* ----- Fastled constants ----- */

const uint8_t kPinLedStrip = 22; // M5StickC grove port, yellow cable

const uint8_t kNumLeds = 60;

const uint8_t kLedStripBrightness = 150;



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

//const uint8_t kFreqBandCount = 7;
//const float kFreqBandStartHz = 20;
//const float kFreqBandEndHz[kFreqBandCount] = {60, 250, 500, 2000, 4000, 6000, 20000};

const uint8_t kFreqBandCount = 20;

const float kFreqBandStartHz = 20;

// Index:                                         0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18      19
const float kFreqBandEndHz[kFreqBandCount] = {   30,   50,   75,  100,  140,  180,  225,  270,  350,  440,  550,  700,  900, 1100, 1400, 1800, 2200, 2800, 3550,  18000};
const float kFreqBandAmp[kFreqBandCount]   = { 0.15f, 0.3f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.3f, 0.4f, 0.4f, 0.4f, 0.5f, 0.8f,    1,    1,    1,    1,    1,   0.3f};

fftData_t sensitivityFactor_ = 1;
const float kSensitivityFactorMax = 1000.0f;

float magnitudeBandMax_[kFreqBandCount] = { 0.0f };

/* Start indices, end indices and normalization factors for each frequency band
    The indices relate to the frequency bins resulting from the FFT */
uint16_t freqBandBinIdxStart_[kFreqBandCount] = { 0 };
uint16_t freqBandBinIdxEnd_[kFreqBandCount] = { 0 };
uint16_t freqBandBinCount_[kFreqBandCount] = { 0 };

/* ----- Beat detection constants and variables ----- */

const uint8_t kBeatDetectBand = 2;

const float kBeatThreshold = 4.0f;

float beatHist_[3] = {0.0f};

uint8_t beatVisIntensity_ = 0;

bool setupSpectrumAnalysis()
{
    bool success = true;

    // Variables for bin indices and bin count belonging to the current frequency band
    uint16_t binIdxStart; // Index of the first frequency bin of the current frequency band
    uint16_t binIdxEnd;   // Index of the last frequency bin of the current frequency band
    
    // Set bin index for the start of the first frequency band
    binIdxStart = ceilf( kFreqBandStartHz / kFFT_FreqStep);

    // Compute values for all frequency bands
    for (uint8_t bandIdx = 0; bandIdx < kFreqBandCount; bandIdx++)
    {
        // Store index of first frequency bin of current band
        if ( binIdxStart < kFFT_FreqBinCount )
        {
            freqBandBinIdxStart_[bandIdx] = binIdxStart;
        }
        else
        {
            freqBandBinIdxStart_[bandIdx] = 0;
            
            success = false;

            log_e("Failed to set start bin index for frequency band no. %d", bandIdx);
        }
        
        // Compute index of last frequency bin of current band
        binIdxEnd = ceilf( kFreqBandEndHz[bandIdx] / kFFT_FreqStep ) - 1;
        
        if ( binIdxEnd < kFFT_FreqBinCount)
        {
            freqBandBinIdxEnd_[bandIdx] = binIdxEnd;
        }
        else
        {
            freqBandBinIdxEnd_[bandIdx] = 0;
            binIdxEnd = kFFT_FreqBinCount - 1;

            success = false;

            log_e("Failed to set end bin index for frequency band no. %d", bandIdx);
        }

        // Compute bin count for current band
        freqBandBinCount_[bandIdx] = binIdxEnd - binIdxStart + 1;

        // Set binIdxStart for next band
        binIdxStart = binIdxEnd + 1;

        log_d("Bins in band %d: %d to %d. Number of bins: %d.",
            bandIdx,
            freqBandBinIdxStart_[bandIdx], freqBandBinIdxEnd_[bandIdx],
            freqBandBinCount_[bandIdx]);
    }

    return success;
}

bool setupI2Smic()
{
    esp_err_t i2sErr;

    // i2s configuration for sampling 16 bit mono audio data
    //
    // Notes related to i2s.c:
    // - 'dma_buf_len', i.e. the number of samples in each DMA buffer, is limited to 1024
    // - 'dma_buf_len' * 'bytes_per_sample' is limted to 4092
    // - 'I2S_CHANNEL_FMT_ONLY_RIGHT' means "mono", i.e. only one channel to be received via i2s
    //   In the M5StickC microphone example 'I2S_CHANNEL_FMT_ALL_RIGHT' is used which means two channels.
    //   Afterwards, i2s_set_clk is called to change the DMA configuration to just one channel.
    //

  /*  // FROM TESTING FILE
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = kSampleRate,//SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // changed from left
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};
*/
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),// | I2S_MODE_PDM
        .sample_rate = kSampleRate,
        .bits_per_sample = kI2S_BitsPerSample,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = kI2S_BufferCount,
        .dma_buf_len = kI2S_BufferSizeSamples
    };

    i2sErr = i2s_driver_install(I2S_NUM_0, &i2sConfig, kI2S_QueueLength, &pI2S_Queue_);

    if (i2sErr)
    {
        log_e("Failed to start i2s driver. ESP error: %s (%x)", esp_err_to_name(i2sErr), i2sErr);
        return false;
    }

    if (pI2S_Queue_ == nullptr)
    {
        log_e("Failed to setup i2s event queue.");
        return false;
    }

    // Configure i2s pins for sampling audio data from the built-in microphone of the M5StickC

    i2s_pin_config_t i2sPinConfig = {
        .bck_io_num = I2S_MIC_SERIAL_CLOCK,
        .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_MIC_SERIAL_DATA};

    i2sErr = i2s_set_pin(I2S_NUM_0, &i2sPinConfig);

    if (i2sErr)
    {
        log_e("Failed to set i2s pins. ESP error: %s (%x)", esp_err_to_name(i2sErr), i2sErr);
        return false;
    }

    return true;
}
