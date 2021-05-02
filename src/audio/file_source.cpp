#include "file_source.h"

#include <algorithm>  // min, max
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

FileAudioSource::FileAudioSource(std::string filename) {
    int status;
    AVFormatContext *format;
    AVCodecContext *context;
    AVStream *stream;
    AVCodec *codec;

    // Open file
    format = avformat_alloc_context();
    status = avformat_open_input(&format, filename.c_str(), NULL, NULL);
    if (status != 0) {
        avformat_free_context(format);
        throw AudioSourceError(status, "avformat_open_input", "");
    }

    // Detect streams
    status = avformat_find_stream_info(format, NULL);
    if (status < 0) {
        avformat_close_input(&format);
        avformat_free_context(format);
        throw AudioSourceError(status, "avformat_find_stream_info", "");
    }

    // Determine best stream
    status = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (status == AVERROR_STREAM_NOT_FOUND) {
        avformat_close_input(&format);
        avformat_free_context(format);
        throw AudioSourceError(status, "av_find_best_stream",
                               "Stream not found");
    }
    if (status == AVERROR_DECODER_NOT_FOUND) {
        avformat_close_input(&format);
        avformat_free_context(format);
        throw AudioSourceError(status, "av_find_best_stream",
                               "Decoder not found");
    }

    // Set selected stream
    int streamidx = status;
    stream = format->streams[streamidx];
    stream->need_parsing = AVSTREAM_PARSE_TIMESTAMPS;

    // Set stream properties
    num_channels_ = stream->codecpar->channels;
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

    // Setup decoder
    context = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(context, stream->codecpar);
    status = avcodec_open2(context, codec, NULL);
    if (status < 0) {
        avcodec_free_context(&context);
        avformat_close_input(&format);
        avformat_free_context(format);
        throw AudioSourceError(status, "avcodec_open2", "");
    }

    // prepare resampler
    struct SwrContext *swr = swr_alloc();
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
    if (!swr_is_initialized(swr)) {
        swr_free(&swr);
        avcodec_close(context);
        avcodec_free_context(&context);
        avformat_close_input(&format);
        avformat_free_context(format);
        throw AudioSourceError(-1, "swr_init",
                               "Resampler couldn't be initialized");
    }

    // prepare to read data
    AVPacket *packet = av_packet_alloc();
    av_init_packet(packet);

    AVFrame *frame = av_frame_alloc();
    AVFrame *outframe = av_frame_alloc();

    if (!frame || !outframe) {
        throw AudioSourceError(-1, "av_frame_alloc", "Couldn't allocate frame");
    }

    // iterate through frames
    unsigned long oldsize = 0;
    unsigned long newsize = 0;
    while (av_read_frame(format, packet) >= 0) {
        if (packet->stream_index != streamidx) {
            continue;
        }

        status = avcodec_send_packet(context, packet);
        if (status < 0) {
            throw AudioSourceError(status, "avcodec_send_packet",
                                   "Packet error");
        }

        status = avcodec_receive_frame(context, frame);
        if (status < 0) {
            throw AudioSourceError(status, "avcodec_receive_frame",
                                   "Frame error");
        }

        av_frame_copy_props(outframe, frame);
        outframe->channel_layout = AV_CH_LAYOUT_STEREO;
        outframe->format = AV_SAMPLE_FMT_FLT;
        outframe->sample_rate = frame->sample_rate;

        status = swr_convert_frame(swr, outframe, frame);
        if (status != 0) {
            throw AudioSourceError(status, "swr_convert_frame",
                                   "Resample error");
        }

        oldsize = newsize;
        newsize = oldsize + outframe->nb_samples * outframe->channels;

        // Expand array and copy frame contents
        data_.resize(newsize);
        memcpy(data_.data() + oldsize, outframe->data[0],
               outframe->nb_samples * outframe->channels * sizeof(float));

        // Close packet/frame
        av_frame_unref(outframe);
        av_frame_unref(frame);
        av_packet_unref(packet);
    }

    num_samples_ = newsize / num_channels_;

    // clean up
    av_frame_free(&outframe);
    av_frame_free(&frame);
    av_packet_free(&packet);

    swr_close(swr);
    swr_free(&swr);

    avcodec_close(context);
    avcodec_free_context(&context);

    avformat_close_input(&format);
    avformat_free_context(format);
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

AudioSourceError::AudioSourceError(int error_code, std::string error_source,
                                   std::string info) {
    std::ostringstream os;
    os << "Failed to open audio source due to error ";
    os << std::to_string(error_code) << " returned by " << error_source;
    if (!info.empty()) os << " (" << info << ")";
    message = os.str();
}
