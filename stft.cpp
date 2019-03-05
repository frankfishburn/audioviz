#include <algorithm>  // min,max
#include <cmath>      // INFINITY
#include <cstdio>     // printf

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
    freq_len = spectrogram_get_freqlen(program);

    // Get frequency vector
    freq.resize(freq_len);
    spectrogram_get_freq(program, (void*)freq.data());

    // Populate the output frequency vector
    out_freq_len    = 1000;
    float min_freq  = 25;
    float max_freq  = 7902;
    float increment = (log2(max_freq) - log2(min_freq)) / (out_freq_len - 1);
    out_freq.resize(out_freq_len);
    for (int i = 0; i < out_freq_len; i++) {
        out_freq[i] = pow(2, log2(min_freq) + i * increment);
    }

    // Allocate spectrogram power for each channel
    out_power.resize(num_channels);
    power.resize(num_channels);
    for (int i = 0; i < num_channels; i++) {
        out_power[i].resize(out_freq_len);
        std::fill(out_power[i].begin(), out_power[i].end(), 0);

        power[i].resize(freq_len);
        std::fill(power[i].begin(), power[i].end(), 0);
    }

    // Setup interpolator
    interpolator = new interpolant((int)freq_len, freq.data(), out_freq_len, out_freq.data());
}

void STFT::analyze() {
    maxpower          = 1.0f;
    float tmpmaxpower = -INFINITY;

    for (int idx = 0; idx < 100; idx++) {
        long start_index = (long)round((idx / 100.0) * num_samples);

        for (int channel = 0; channel < num_channels; channel++) {
            // Compute spectrogram
            compute(channel, start_index);

            // Get maximum power
            for (int freq = 0; freq < out_freq_len; freq++) {
                tmpmaxpower = std::max(tmpmaxpower, out_power[channel][freq]);
            }
        }
    }

    maxpower = tmpmaxpower;
}

void STFT::compute(const int channel, const long sample_index) {
    const long max_sample_index = num_samples - props.num_samples - 1;

    long start_index = sample_index - props.num_samples / 2;
    start_index      = std::max(start_index, (long)0);
    start_index      = std::min(start_index, max_sample_index);

    spectrogram_execute(program, (void*)(audio_ptr + num_channels * start_index + channel));
    spectrogram_get_power_periodogram(program, (void*)power[channel].data());

    // Rescale based on frequency and log transform
    for (unsigned long fidx = 0; fidx < freq_len; fidx++) {
        float scale          = 1 / (48.35 * pow(log2(freq[fidx]), -3.434));
        power[channel][fidx] = sqrt(power[channel][fidx]) * scale / maxpower;
    }

    interpolator->estimate(power[channel].data(), out_power[channel].data());
}

int STFT::maxGoodFreq() {
    return out_freq_len;
}