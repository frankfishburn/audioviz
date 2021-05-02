#ifndef AUDIO_SOURCE_H
#define AUDIO_SOURCE_H

#include <sstream>
#include <string>
#include <vector>

class AudioSource {
   public:
    AudioSource() {};
    AudioSource(std::string filename);

    // Query state
    bool loaded() { return loaded_; };

    // Query audio file properties
    unsigned long num_channels() const { return num_channels_; };
    unsigned long num_samples() const { return num_samples_; };
    unsigned long sample_rate() const { return sample_rate_; };
    std::string info() const;

    // Get pointer to audio data
    float* data() { return data_.data(); };

    // Query metadata
    std::string artist() const { return artist_; };
    std::string album() const { return album_; };
    std::string title() const { return title_; };
    std::string year() const { return year_; };
    std::string description () const;

   private:
    // State
    bool loaded_ = false;

    // Stream properties
    unsigned long num_channels_;
    unsigned long num_samples_;
    unsigned long sample_rate_;
    
    // Interleaved audio data
    std::vector<float> data_;

    // Metadata
    std::string filename_;
    std::string artist_;
    std::string album_;
    std::string title_;
    std::string year_;

};


class AudioSourceError : virtual public std::exception {
public:
    AudioSourceError(int error_code, std::string error_source, std::string info);
    const char * what () const throw () {return message.c_str();}
protected:
    std::string message;
};

#endif /* AUDIO_SOURCE_H */
