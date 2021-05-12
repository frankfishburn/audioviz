#ifndef FILE_AUDIO_SOURCE_H
#define FILE_AUDIO_SOURCE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "i_source.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

class FileAudioSource : public IAudioSource {
   public:
    FileAudioSource();
    ~FileAudioSource();
    void open(std::string filename);

    // Query state
    bool loaded() const { return loaded_; };

    // Query audio file properties
    unsigned long num_channels() const { return num_channels_; };
    unsigned long num_samples() const { return num_samples_; };
    unsigned long sample_rate() const { return sample_rate_; };
    std::string info() const;

    // Get pointer to audio data
    const std::vector<float> &data() const { return data_; };
    std::vector<float> get_segment(const int channel, const long center,
                                   const long width) const;

    // Query metadata
    std::string description() const;

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
    std::unordered_map<std::string, std::string> tags_;

    // Parsing data structures
    AVFormatContext *format;
    AVCodecContext *context;
    SwrContext *swr;
    AVPacket *packet;
    AVFrame *in_frame;
    AVFrame *out_frame;
};

class AudioSourceError : virtual public std::exception {
   public:
    AudioSourceError(int error_code, std::string error_source,
                     std::string info);
    const char *what() const throw() { return message.c_str(); }

   protected:
    std::string message;
};

#endif /* FILE_AUDIO_SOURCE_H */
