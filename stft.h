#ifndef STFT_H
#define STFT_H

#include <vector>
#include "audio_manager.h"
#include "interpolant.h"
#include "spectrogram.h"

class STFT {
   public:
    STFT(audio_manager& audio, SpectrogramConfig& conf, unsigned long insamples);
    STFT(const STFT& orig);
    ~STFT();

    // methods
    void initialize();
    void analyze();
    void compute(const int channel, const long sample_index);

    // accessors
    float*        getPowerPtr(const int channel) { return out_power[channel].data(); };
    unsigned long numSamples() { return props.num_samples; };
    unsigned long numTime() { return time_len; };
    unsigned long numFreq() { return freq_len; };
    int           maxGoodFreq();

   private:
    int           num_channels;
    unsigned long num_samples;
    float*        audio_ptr;

    SpectrogramInput      props;
    SpectrogramConfig     config;
    SpectrogramTransform* program;

    unsigned long                   freq_len;
    unsigned long                   time_len;
    std::vector<float>              freq;
    std::vector<float>              time;
    std::vector<std::vector<float>> power;

    int                             out_freq_len;
    std::vector<float>              out_freq;
    std::vector<std::vector<float>> out_power;
    interpolant*                    interpolator;

    float maxpower = 1.0;
};

#endif /* STFT_H */
