#include "Audio.h"
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

void newAudioReading(float *magnitudeBand, float *magnitudeBandWeightedMax){
    size_t i2sBytesRead = 0;

    i2s_read(I2S_NUM_0, raw_samples, sizeof(int16_t) * kFFT_SampleCount, &i2sBytesRead, portMAX_DELAY); // delay on this rougly 28ms
    int max_amp = 0;
    int samples_read = i2sBytesRead / sizeof(int16_t);

    int32_t blockSum = raw_samples[0];

    for (int i = 1; i < kFFT_SampleCount; i++)
    {
        blockSum += raw_samples[i];
    }

    int16_t blockAvg = blockSum / kFFT_SampleCount;
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
        if ( magnitudeBandWeighted > *magnitudeBandWeightedMax )
        {
            *magnitudeBandWeightedMax = magnitudeBandWeighted;
        }
    }
    
    // Update the sensitivity factor
    const float s1 = 8.0f / 1024.0f;
    const float s2 = 1.0f - s1;
    sensitivityFactor_ =  min( (250.0f / *magnitudeBandWeightedMax) * s1 + sensitivityFactor_ * s2, kSensitivityFactorMax );

}