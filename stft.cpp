#include <algorithm>  // min,max
#include <cmath>      // INFINITY

#ifndef SAVE_STFT
#define SAVE_STFT 0
#endif

#if SAVE_STFT == 1
#include <fstream>  // write csv
#endif

#include "stft.h"

STFT::STFT(audio_manager& audio, SpectrogramConfig& inputconfig, unsigned long input_samples) {
    num_channels = audio.get_num_channels();
    num_samples  = audio.get_num_samples();
    audio_ptr    = audio.get_data();

    // Configure input data properties
    props.data_size   = sizeof(float);
    props.sample_rate = audio.get_sample_rate();
    props.num_samples = input_samples;
    props.stride      = num_channels;

    // Configure spectrogram transform
    config.padding_mode     = inputconfig.padding_mode;
    config.window_length    = inputconfig.window_length;
    config.window_overlap   = inputconfig.window_overlap;
    config.transform_length = inputconfig.transform_length;
    config.window_type      = inputconfig.window_type;

    initialize();
}

STFT::STFT(const STFT& orig) {
    // Copy parameters
    num_channels = orig.num_channels;
    num_samples  = orig.num_samples;
    audio_ptr    = orig.audio_ptr;

    props  = orig.props;
    config = orig.config;

    initialize();
}

STFT::~STFT() {
    delete interpolator;
    spectrogram_destroy(program);
}

void STFT::initialize() {
    // Create spectrogram object
    program = spectrogram_create(&props, &config);

    // Get derived output dimensions
    temp_freq_len = spectrogram_get_freqlen(program);

    // Populate the intermediate results
    temp_freq.resize(temp_freq_len);
    spectrogram_get_freq(program, (void*)temp_freq.data());

    temp_power.resize(temp_freq_len);

    // Initialize output
    result.resize(num_channels);
    for (int ch = 0; ch < num_channels; ch++) {
        // Setup
        float freq_length = 1000;
        float min_freq    = 25;
        float max_freq    = 7902;
        float increment   = (log2(max_freq) - log2(min_freq)) / (freq_length - 1);

        // Allocate
        result[ch].length = freq_length;
        result[ch].freq.resize(freq_length);
        result[ch].power.resize(freq_length);

        // Populate frequency
        for (int i = 0; i < freq_length; i++) {
            result[ch].freq[i] = pow(2, log2(min_freq) + i * increment);
        }
    }

    // Setup interpolator
    interpolator = new interpolant((int)temp_freq_len, temp_freq.data(), result[0].length, result[0].freq.data());
}

void STFT::analyze() {
    maxmaxpower               = 1.0f;
    maxsumpower               = 1.0f;
    maxdeltasumpower          = 1.0f;
    float tmpmaxmaxpower      = -INFINITY;  // Max. individual power value
    float tmpmaxsumpower      = -INFINITY;  // Max. sum of power over all frequencies
    float tmpmaxdeltasumpower = -INFINITY;  // Max. change in sum of power over all frequencies

    const int analysis_len = 100;

#if SAVE_STFT == 1
    // Open debugging output CSV files
    std::ofstream writer[num_channels];
    for (int channel = 0; channel < num_channels; channel++) {
        char filename[50];
        sprintf(filename, "audioviz_analysis_%i.csv", channel);
        writer[channel].open(filename);
    }
#endif

    float prev_sumpower = 0;
    for (int idx = 0; idx < analysis_len; idx++) {
        long start_index = (long)round((idx / (float)analysis_len) * num_samples);

        for (int ch = 0; ch < num_channels; ch++) {
            prev_sumpower = result[ch].sumpower;

            // Compute spectrogram
            compute(ch, start_index);

#if SAVE_STFT == 1
            // Save power spectrum
            for (int freq = 0; freq < result[ch].length; freq++) {
                // Write data to CSV file
                writer[ch] << result[ch].power[freq];
                if (freq < result[ch].length - 1) {
                    writer[ch] << ", ";
                }
            }
            writer[ch] << "\n";
#endif

            tmpmaxmaxpower      = std::max(tmpmaxmaxpower, result[ch].maxpower);
            tmpmaxsumpower      = std::max(tmpmaxsumpower, result[ch].sumpower);
            tmpmaxdeltasumpower = std::max(tmpmaxdeltasumpower, std::abs(result[ch].sumpower - prev_sumpower));
        }
    }

#if SAVE_STFT == 1
    // Close debugging output CSV files
    for (int channel = 0; channel < num_channels; channel++) {
        writer[channel].close();
    }
#endif

    maxmaxpower      = tmpmaxmaxpower;
    maxsumpower      = tmpmaxsumpower;
    maxdeltasumpower = tmpmaxdeltasumpower;
}

void STFT::compute(const long sample_index) {
    for (int ch = 0; ch < num_channels; ch++) {
        compute(ch, sample_index);
    }
}

void STFT::compute(const int channel, const long sample_index) {
    const long max_sample_index = num_samples - props.num_samples - 1;

    long start_index = sample_index - props.num_samples / 2;
    start_index      = std::max(start_index, (long)0);
    start_index      = std::min(start_index, max_sample_index);

    spectrogram_execute(program, (void*)(audio_ptr + num_channels * start_index + channel));
    spectrogram_get_power_periodogram(program, (void*)temp_power.data());

    // Rescale based on frequency and log transform
    for (unsigned long fidx = 0; fidx < temp_freq_len; fidx++) {
        float scale      = 1 / (48.35 * pow(log2(temp_freq[fidx]), -3.434));
        temp_power[fidx] = sqrt(temp_power[fidx]) * scale / maxmaxpower;
    }

    interpolator->estimate(temp_power.data(), result[channel].power.data());

    // Get the sum of power and change
    float maxpower      = -INFINITY;
    float sumpower      = 0;
    float sumpower_prev = result[channel].sumpower;

    for (int fidx = 0; fidx < result[channel].length; fidx++) {
        sumpower += result[channel].power[fidx];
        maxpower = std::max(maxpower, result[channel].power[fidx]);
    }

    result[channel].maxpower      = maxpower / maxmaxpower;
    result[channel].sumpower      = sumpower / maxsumpower;
    result[channel].deltasumpower = (sumpower - sumpower_prev) / maxdeltasumpower;
}

int STFT::maxGoodFreq() {
    return result[0].length;
}