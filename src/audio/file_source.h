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
    bool loaded() const override { return loaded_; };

    // Query audio file properties
    unsigned long num_channels() const override { return num_channels_; };
    unsigned long num_samples() const override { return num_samples_; };
    unsigned long sample_rate() const override { return sample_rate_; };
    std::string info() const override;

    // Get pointer to audio data
    const std::vector<float> &data() const override { return data_; };
    std::vector<float> get_segment(const int channel, const long center,
                                   const long width) const override;

    // Query metadata
    std::string description() const override;

   private:
    // State
    bool loaded_ = false;

    // Stream properties
    unsigned long num_channels_ = 0;
    unsigned long num_samples_ = 0;
    unsigned long sample_rate_ = 0;

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
    AudioSourceError(int error_code, const std::string &error_source,
                     std::string info);
    const char *what() const throw() { return message.c_str(); }

   protected:
    std::string message;
};

#endif /* FILE_AUDIO_SOURCE_H */
