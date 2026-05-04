#include "Audio.h"

#ifdef AUDIO_DISABLED

void setupAudioData() {}
bool setupI2Smic()            { return false; }
bool setupSpectrumAnalysis()  { return false; }
void newAudioReading(float *, float *) {}

#else

bool setupSpectrumAnalysis()
{
    bool success = true;

    uint16_t binIdxStart;
    uint16_t binIdxEnd;

    binIdxStart = ceilf(kFreqBandStartHz / FFT_FREQ_STEP);

    for (uint8_t bandIdx = 0; bandIdx < FREQ_BAND_COUNT; bandIdx++)
    {
        if (binIdxStart < FFT_FREQ_BIN_COUNT)
        {
            freqBandBinIdxStart_[bandIdx] = binIdxStart;
        }
        else
        {
            freqBandBinIdxStart_[bandIdx] = 0;
            success = false;
            log_e("Failed to set start bin index for frequency band no. %d", bandIdx);
        }

        binIdxEnd = ceilf(kFreqBandEndHz[bandIdx] / ((fftData_t)FFT_FREQ_STEP)) - 1;

        if (binIdxEnd < FFT_FREQ_BIN_COUNT)
        {
            freqBandBinIdxEnd_[bandIdx] = binIdxEnd;
        }
        else
        {
            freqBandBinIdxEnd_[bandIdx] = 0;
            binIdxEnd = FFT_FREQ_BIN_COUNT - 1;
            success = false;
            log_e("Failed to set end bin index for frequency band no. %d", bandIdx);
        }

        freqBandBinCount_[bandIdx] = binIdxEnd - binIdxStart + 1;
        binIdxStart = binIdxEnd + 1;

        log_d("Bins in band %d: %d to %d. Number of bins: %d.",
            bandIdx,
            freqBandBinIdxStart_[bandIdx], freqBandBinIdxEnd_[bandIdx],
            freqBandBinCount_[bandIdx]);
    }

    return success;
}

void setupAudioData() {
    raw_samples  = new int16_t[FFT_SAMPLE_COUNT];
    fftDataReal_ = new double[FFT_SAMPLE_COUNT];
    fftDataImag_ = new double[FFT_SAMPLE_COUNT];
    fft_ = arduinoFFT(fftDataReal_, fftDataImag_, FFT_SAMPLE_COUNT, kFFT_SamplingFreq);
}

bool setupI2Smic()
{
    esp_err_t i2sErr;

    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = kI2S_BitsPerSample,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = kI2S_BufferCount,
        .dma_buf_len = kI2S_BufferSizeSamples};

    i2sErr = i2s_driver_install(I2S_NUM_0, &i2sConfig, kI2S_QueueLength, &pI2S_Queue_);
    if (i2sErr) {
        log_e("Failed to start i2s driver. ESP error: %s (%x)", esp_err_to_name(i2sErr), i2sErr);
        return false;
    }

    if (pI2S_Queue_ == nullptr) {
        log_e("Failed to setup i2s event queue.");
        return false;
    }

    i2s_pin_config_t i2sPinConfig = {
        .bck_io_num  = I2S_MIC_SERIAL_CLOCK,
        .ws_io_num   = I2S_MIC_LEFT_RIGHT_CLOCK,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num  = I2S_MIC_SERIAL_DATA};

    i2sErr = i2s_set_pin(I2S_NUM_0, &i2sPinConfig);
    if (i2sErr) {
        log_e("Failed to set i2s pins. ESP error: %s (%x)", esp_err_to_name(i2sErr), i2sErr);
        return false;
    }

    return true;
}

void newAudioReading(float *magnitudeBand, float *magnitudeBandWeightedMax)
{
    size_t i2sBytesRead = 0;
    i2s_read(I2S_NUM_0, raw_samples, sizeof(int16_t) * FFT_SAMPLE_COUNT, &i2sBytesRead, portMAX_DELAY);
    int samples_read = i2sBytesRead / sizeof(int16_t);

    int32_t blockSum = raw_samples[0];
    for (int i = 1; i < FFT_SAMPLE_COUNT; i++)
        blockSum += raw_samples[i];
    int16_t blockAvg = blockSum / FFT_SAMPLE_COUNT;

    for (int i = 0; i < FFT_SAMPLE_COUNT; i++)
    {
        int16_t v = raw_samples[i] - blockAvg;
        const fftData_t kInt16MaxInv = 1.0f / __INT16_MAX__;
        fftDataReal_[i] = kInt16MaxInv * v;
        fftDataImag_[i] = 0.0f;
    }
    fft_.Compute(FFT_FORWARD);

    fftData_t magnitudeSum = 0;
    for (uint16_t i = 0; i < FFT_FREQ_BIN_COUNT; i++)
    {
        float magValNew = sqrtf(fftDataReal_[i] * fftDataReal_[i] + fftDataImag_[i] * fftDataImag_[i]);
        const float w1 = 16.0f / 128.0f;
        const float w2 = 1 - w1;
        magnitudeSpectrumAvg_[i] = magValNew * w1 + magnitudeSpectrumAvg_[i] * w2;
        magnitudeSum += magnitudeSpectrumAvg_[i];
    }

    for (uint8_t bandIdx = 0; bandIdx < FREQ_BAND_COUNT; bandIdx++)
    {
        for (uint16_t binIdx = freqBandBinIdxStart_[bandIdx]; binIdx <= freqBandBinIdxEnd_[bandIdx]; binIdx++)
        {
            if (magnitudeSpectrumAvg_[binIdx] > magnitudeBand[bandIdx])
                magnitudeBand[bandIdx] = magnitudeSpectrumAvg_[binIdx];
        }

        float magnitudeBandWeighted = magnitudeBand[bandIdx] * kFreqBandAmp[bandIdx];

        if (magnitudeBand[bandIdx] > magnitudeBandMax_[bandIdx])
            magnitudeBandMax_[bandIdx] = magnitudeBand[bandIdx];

        if (magnitudeBandWeighted > *magnitudeBandWeightedMax)
            *magnitudeBandWeightedMax = magnitudeBandWeighted;
    }

    const float s1 = 8.0f / 1024.0f;
    const float s2 = 1.0f - s1;
    sensitivityFactor_ = min((250.0f / *magnitudeBandWeightedMax) * s1 + sensitivityFactor_ * s2, kSensitivityFactorMax);
}

#endif
