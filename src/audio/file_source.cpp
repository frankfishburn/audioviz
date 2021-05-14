#include "file_source.h"

#include <algorithm>  // min, max
#include <sstream>

FileAudioSource::FileAudioSource() {
    format = avformat_alloc_context();
    context = avcodec_alloc_context3(NULL);
    swr = swr_alloc();
    packet = av_packet_alloc();
    in_frame = av_frame_alloc();
    out_frame = av_frame_alloc();
}

FileAudioSource::~FileAudioSource() {
    av_frame_free(&out_frame);
    av_frame_free(&in_frame);
    av_packet_free(&packet);

    swr_close(swr);
    swr_free(&swr);

    avcodec_close(context);
    avcodec_free_context(&context);

    avformat_close_input(&format);
    avformat_free_context(format);
}

void FileAudioSource::open(std::string filename) {
    int status;
    AVStream *stream;
    AVCodec *codec;

    // Open file
    status = avformat_open_input(&format, filename.c_str(), NULL, NULL);
    if (status != 0) throw AudioSourceError(status, "avformat_open_input", "");

    // Detect streams
    status = avformat_find_stream_info(format, NULL);
    if (status < 0)
        throw AudioSourceError(status, "avformat_find_stream_info", "");

    // Determine best stream
    status = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (status == AVERROR_STREAM_NOT_FOUND)
        throw AudioSourceError(status, "av_find_best_stream",
                               "Stream not found");
    if (status == AVERROR_DECODER_NOT_FOUND)
        throw AudioSourceError(status, "av_find_best_stream",
                               "Decoder not found");

    // Set selected stream
    const int stream_index = status;
    stream = format->streams[stream_index];
    stream->need_parsing = AVSTREAM_PARSE_TIMESTAMPS;

    // Setup decoder
    avcodec_parameters_to_context(context, stream->codecpar);
    status = avcodec_open2(context, codec, NULL);
    if (status < 0) throw AudioSourceError(status, "avcodec_open2", "");

    // prepare resampler
    av_opt_set_int(swr, "in_channel_count", stream->codecpar->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 2, 0);
    av_opt_set_int(swr, "in_channel_layout", stream->codecpar->channel_layout,
                   0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr, "in_sample_rate", stream->codecpar->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", stream->codecpar->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", context->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
    swr_init(swr);
    if (!swr_is_initialized(swr))
        throw AudioSourceError(-1, "swr_init",
                               "Resampler couldn't be initialized");

    // prepare to read data
    av_init_packet(packet);

    // iterate through frames
    unsigned long oldsize = 0;
    unsigned long newsize = 0;
    while (av_read_frame(format, packet) >= 0) {
        if (packet->stream_index != stream_index) {
            continue;
        }

        status = avcodec_send_packet(context, packet);
        if (status < 0)
            throw AudioSourceError(status, "avcodec_send_packet",
                                   "Packet error");

        status = avcodec_receive_frame(context, in_frame);
        if (status < 0)
            throw AudioSourceError(status, "avcodec_receive_frame",
                                   "Frame error");

        av_frame_copy_props(out_frame, in_frame);
        out_frame->channel_layout = AV_CH_LAYOUT_STEREO;
        out_frame->format = AV_SAMPLE_FMT_FLT;
        out_frame->sample_rate = in_frame->sample_rate;

        status = swr_convert_frame(swr, out_frame, in_frame);
        if (status != 0)
            throw AudioSourceError(status, "swr_convert_frame",
                                   "Resample error");

        oldsize = newsize;
        newsize = oldsize + out_frame->nb_samples * out_frame->channels;

        // Expand array and copy frame contents
        data_.resize(newsize);
        memcpy(data_.data() + oldsize, out_frame->data[0],
               out_frame->nb_samples * out_frame->channels * sizeof(float));

        // Close packet/frame
        av_frame_unref(out_frame);
        av_frame_unref(in_frame);
        av_packet_unref(packet);
    }

    // Set stream properties
    num_channels_ = stream->codecpar->channels;
    num_samples_ = newsize / num_channels_;
    sample_rate_ = stream->codecpar->sample_rate;
    filename_ = filename;
    loaded_ = true;

    // Update audio metadata
    AVDictionaryEntry *tag = NULL;
    while (true) {
        tag = av_dict_get(format->metadata, "", tag, AV_DICT_IGNORE_SUFFIX);
        if (tag == NULL) break;
        tags_.insert({tag->key, tag->value});
    }
}

std::vector<float> FileAudioSource::get_segment(const int channel,
                                                const long center,
                                                const long width) const {
    unsigned int real_channel = channel;
    if (real_channel > num_channels_) real_channel = 0;

    // Check bounds of window
    long start = center - width / 2;
    start = std::max(start, (long)0);
    start = std::min(start, (long)num_samples_ - width - 1);

    long end = start + width;
    end = std::min(end, (long)num_samples_);

    long real_width = end - start;

    // Create output
    std::vector<float> window(real_width);
    for (long idx = 0; idx < real_width; idx++) {
        long sample = start + idx;
        window[idx] = data_[num_channels_ * sample + real_channel];
    }

    return window;
}

std::string FileAudioSource::info() const {
    std::ostringstream os;
    os << "Filename:      " << filename_ << std::endl;
    os << "# of samples:  " << num_samples_ << std::endl;
    os << "# of channels: " << num_channels_ << std::endl;
    os << "Sample rate:   " << sample_rate_ << std::endl;
    return os.str();
}

std::string FileAudioSource::description() const {
    std::ostringstream os;
    if (tags_.find("title") != tags_.end()) {
        os << "\"" << tags_.at("title") << "\"";
    } else {
        os << "<Unknown>";
    }
    if (tags_.find("artist") != tags_.end()) {
        os << " by \"" << tags_.at("artist") << "\"";
    } else {
        os << " by <Unknown>";
    }
    if (tags_.find("album") != tags_.end()) {
        os << " from \"" << tags_.at("album") << "\"";
    }
    if (tags_.find("originalyear") != tags_.end()) {
        os << " (" << tags_.at("originalyear") << ")";
    }
    return os.str();
}

AudioSourceError::AudioSourceError(int error_code,
                                   const std::string &error_source,
                                   std::string info) {
    std::ostringstream os;
    os << "Failed to open audio source due to error ";
    os << std::to_string(error_code) << " returned by " << error_source;
    if (!info.empty()) os << " (" << info << ")";
    message = os.str();
}
