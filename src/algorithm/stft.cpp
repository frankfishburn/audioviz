#include "stft.h"

#include <algorithm>  // min,max
#include <cmath>      // INFINITY

// Spectrum resampling parameters
static constexpr float num_freq = 1000;
static constexpr float min_freq = 25;
static constexpr float max_freq = 7902;

STFT::STFT(SpectrogramInput props, SpectrogramConfig config)
    : props_(props), config_(config) {
    // Create spectrogram object
    transform_ = spectrogram_create(&props, &config);

    // Get derived output dimensions
    num_raw_frequencies_ = spectrogram_get_freqlen(transform_);

    // Allocate raw spectra
    raw_frequencies_.resize(num_raw_frequencies_);
    raw_power_.resize(num_raw_frequencies_);
    spectrogram_get_freq(transform_, (void *)raw_frequencies_.data());

    // Allocate resampled spectra
    std::vector<float> frequencies(num_freq);
    const float increment = (log2(max_freq) - log2(min_freq)) / (num_freq - 1);

    for (int idx = 0; idx < num_freq; idx++)
        frequencies[idx] = pow(2, log2(min_freq) + idx * increment);

    // Setup resampler
    resampler_ = Resampler(raw_frequencies_, frequencies);
}

STFT::~STFT() { spectrogram_destroy(transform_); }

std::vector<float> STFT::compute(std::vector<float> &signal) const {
    if (signal.size() < props_.num_samples) signal.resize(props_.num_samples);

    spectrogram_execute(transform_, (void *)signal.data());
    spectrogram_get_power_periodogram(transform_, (void *)raw_power_.data());

    // Rescale based on frequency and log transform
    for (unsigned long idx = 0; idx < num_raw_frequencies_; idx++) {
        float scale = 1 / (48.35 * pow(log2(raw_frequencies_[idx]), -3.434));
        raw_power_[idx] = sqrt(raw_power_[idx]) * scale;
    }

    return resampler_.resample(raw_power_);
}

unsigned long STFT::length() const { return num_freq; }
