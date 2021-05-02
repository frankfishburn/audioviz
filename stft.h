#ifndef STFT_H
#define STFT_H

#include <vector>
#include "audio_source.h"
#include "interpolant.h"
#include "spectrogram.h"

struct Spectrum {
    int                length;
    std::vector<float> freq;
    std::vector<float> power;
    float              maxpower;
    float              sumpower;
    float              deltasumpower;
};

class STFT {
   public:
    STFT(AudioSource& audio, SpectrogramConfig& conf, unsigned long insamples);
    STFT(const STFT& orig);
    ~STFT();

    // Methods
    void initialize();
    void analyze();
    void compute(const long sample_index);
    void compute(const int channel, const long sample_index);

    // Accessors
    Spectrum      getSpectrum(int ch) { return result[ch]; };
    unsigned int  numChannels() { return num_channels; };
    unsigned long numSamples() { return props.num_samples; };
    unsigned long numFreq() { return result[0].length; };
    int           maxGoodFreq();

   private:
    int           num_channels;
    unsigned long num_samples;
    float*        audio_ptr;

    // Configuration
    SpectrogramInput      props;
    SpectrogramConfig     config;
    SpectrogramTransform* program;
    interpolant*          interpolator;

    // Temporaries
    std::vector<float> temp_freq;
    std::vector<float> temp_power;
    unsigned long      temp_freq_len;

    // Output
    std::vector<Spectrum> result;

    // Internal whole-file properties
    float maxmaxpower      = 1.0;
    float maxsumpower      = 1.0;
    float maxdeltasumpower = 1.0;
};

#endif /* STFT_H */
