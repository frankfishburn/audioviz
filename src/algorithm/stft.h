#ifndef STFT_H
#define STFT_H

#include <spectrogram.h>

#include <vector>

#include "resampler.h"

class STFT {
   public:
    STFT(SpectrogramInput props, SpectrogramConfig conf);
    ~STFT();

    unsigned long length() const;
    std::vector<float> compute(std::vector<float>& signal) const;

   private:
    // Configuration
    const SpectrogramInput props_;
    const SpectrogramConfig config_;
    SpectrogramTransform* transform_;
    Resampler resampler_;

    // Temporary data
    unsigned long num_raw_frequencies_;
    std::vector<float> raw_frequencies_;
    mutable std::vector<float> raw_power_;
};

#endif /* STFT_H */
