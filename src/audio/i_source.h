#ifndef I_AUDIO_SOURCE_H
#define I_AUDIO_SOURCE_H

#include <string>
#include <vector>

class IAudioSource {
   public:
    virtual ~IAudioSource(){};

    // Query state
    virtual bool loaded() const = 0;

    // Query audio file properties
    virtual unsigned long num_channels() const = 0;
    virtual unsigned long num_samples() const = 0;
    virtual unsigned long sample_rate() const = 0;
    virtual std::string info() const = 0;

    // Get pointer to audio data
    virtual const std::vector<float> &data() const = 0;
    virtual std::vector<float> get_segment(const int channel, const long center,
                                           const long width) const = 0;

    // Query metadata
    virtual std::string description() const = 0;
};

#endif /* I_AUDIO_SOURCE_H */
