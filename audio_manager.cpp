#include "audio_manager.h"

#include <SDL2/SDL.h>  // Audio playback
#include <algorithm>   // min, max

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

audio_manager::audio_manager(const char* filename) {
    input_file = std::string(filename);
    load_file();
    setup_playback();
}

audio_manager::audio_manager(const audio_manager& orig) {
    data.resize(orig.num_samples * orig.num_channels);
    memcpy(data.data(), orig.data.data(), num_samples * num_channels * sizeof(float));
}

audio_manager::~audio_manager() {
    SDL_CloseAudio();

    if (isLoaded) {
        isLoaded   = false;
        isPlayable = false;
    }
}

void audio_manager::load_file() {
    int              status;
    char             errmsg[256];
    const char*      filename = input_file.c_str();
    AVFormatContext* format;
    AVCodecContext*  context;
    AVStream*        stream;
    AVCodec*         codec;

    // Open file
    format = avformat_alloc_context();
    status = avformat_open_input(&format, filename, NULL, NULL);
    if (status != 0) {
        avformat_free_context(format);
        fprintf(stderr, "Could not open file '%s'\n", filename);
        fflush(stderr);
        return;
    }

    // Detect streams
    status = avformat_find_stream_info(format, NULL);
    if (status < 0) {
        avformat_close_input(&format);
        avformat_free_context(format);
        av_strerror(status, errmsg, 256);
        fprintf(stderr, "Could not retrieve stream info (%s)\n", errmsg);
        fflush(stderr);
        return;
    }

    // Determine best stream
    status = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (status == AVERROR_STREAM_NOT_FOUND) {
        avformat_close_input(&format);
        avformat_free_context(format);
        fprintf(stderr, "No audio stream found in file '%s'\n", filename);
        fflush(stderr);
        return;
    }
    if (status == AVERROR_DECODER_NOT_FOUND) {
        avformat_close_input(&format);
        avformat_free_context(format);
        fprintf(stderr, "No decoder found for audio stream in file '%s'\n", filename);
        fflush(stderr);
        return;
    }

    // Set selected stream
    int streamidx        = status;
    stream               = format->streams[streamidx];
    stream->need_parsing = AVSTREAM_PARSE_TIMESTAMPS;
    num_channels         = stream->codecpar->channels;
    sample_rate          = stream->codecpar->sample_rate;

    // Read the metadata
    AVDictionaryEntry* tag = NULL;

    tag = av_dict_get(format->metadata, "artist", NULL, 0);
    if (tag != NULL)
        artist = std::string(tag->value);

    tag = av_dict_get(format->metadata, "album", NULL, 0);
    if (tag != NULL)
        album = std::string(tag->value);

    tag = av_dict_get(format->metadata, "title", NULL, 0);
    if (tag != NULL)
        title = std::string(tag->value);

    tag = av_dict_get(format->metadata, "originalyear", NULL, 0);
    if (tag != NULL)
        year = std::string(tag->value);

    // Setup decoder
    context = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(context, stream->codecpar);
    status = avcodec_open2(context, codec, NULL);
    if (status < 0) {
        avcodec_free_context(&context);
        avformat_close_input(&format);
        avformat_free_context(format);
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n", stream->id, filename);
        fflush(stderr);
        return;
    }

    // prepare resampler
    struct SwrContext* swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count", stream->codecpar->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 2, 0);
    av_opt_set_int(swr, "in_channel_layout", stream->codecpar->channel_layout, 0);
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
        fprintf(stderr, "Resampler could not be initialized\n");
        return;
    }

    // prepare to read data
    AVPacket* packet = av_packet_alloc();
    av_init_packet(packet);

    AVFrame* frame    = av_frame_alloc();
    AVFrame* outframe = av_frame_alloc();

    if (!frame || !outframe) {
        fprintf(stderr, "Error allocating the frame\n");
        return;
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
            av_strerror(status, errmsg, 256);
            fprintf(stderr, "Packet error at index %lu (%s)\n", newsize, errmsg);
            fflush(stderr);
            break;
        }

        status = avcodec_receive_frame(context, frame);
        if (status < 0) {
            av_strerror(status, errmsg, 256);
            fprintf(stderr, "Frame error at index %lu (%s)\n", newsize, errmsg);
            fflush(stderr);
            break;
        }

        av_frame_copy_props(outframe, frame);
        outframe->channel_layout = AV_CH_LAYOUT_STEREO;
        outframe->format         = AV_SAMPLE_FMT_FLT;
        outframe->sample_rate    = frame->sample_rate;

        status = swr_convert_frame(swr, outframe, frame);
        if (status != 0) {
            av_strerror(status, errmsg, 256);
            fprintf(stderr, "Resample error at index %lu (%s)\n", newsize, errmsg);
            fflush(stderr);
            break;
        }

        oldsize = newsize;
        newsize = oldsize + outframe->nb_samples * outframe->channels;

        // Expand array and copy frame contents
        data.resize(newsize);
        memcpy(data.data() + oldsize, outframe->data[0], outframe->nb_samples * outframe->channels * sizeof(float));

        // Close packet/frame
        av_frame_unref(outframe);
        av_frame_unref(frame);
        av_packet_unref(packet);
    }

    num_samples = newsize / num_channels;

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

    printf("Loaded file: %lu channels, %lu samples, %lu Hz\n", num_channels, num_samples, sample_rate);
    fflush(stdout);

    isLoaded = true;
}

void audio_manager::setup_playback() {
    if (!isLoaded)
        return;

    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq     = sample_rate;
    want.format   = AUDIO_F32;
    want.channels = num_channels;
    want.samples  = 8192;
    want.callback = callback;
    want.userdata = (void*)this;

    if (SDL_OpenAudio(&want, &have) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        return;
    } else {
        if (have.format != want.format) {
            SDL_Log("We didn't get Float32 audio format.");
            return;
        }
    }

    isPlayable = true;
}

void audio_manager::callback(void* userdata, Uint8* stream, int len) {
    SDL_memset(stream, 0, len);
    audio_manager* am        = (audio_manager*)userdata;
    Uint8**        audio_ptr = (Uint8**)&(am->data);
    SDL_MixAudioFormat(stream, (Uint8*)*audio_ptr + am->callback_offset, AUDIO_F32, len, SDL_MIX_MAXVOLUME / 2);
    am->callback_offset += len;
}

void audio_manager::update_offset() {
    if (isPlaying) {
        Uint64 now = SDL_GetPerformanceCounter();
        timer_offset += (now - timer_start);
        timer_start = 0;
    }
}

Uint64 audio_manager::get_current_sample() {
    Uint64 current_sample = (Uint64)round(get_current_time() * (double)sample_rate);
    current_sample        = std::min(num_samples, current_sample);
    current_sample        = std::max((Uint64)1, current_sample);
    return current_sample;
}

double audio_manager::get_current_time() {
    Uint64 position = timer_offset;

    if (isPlaying) {
        Uint64 now = SDL_GetPerformanceCounter();
        position += (now - timer_start);
    }

    return position / (double)SDL_GetPerformanceFrequency();
}

std::string audio_manager::get_current_time_str() {
    double seconds = get_current_time();

    int hours = floor(seconds / 3600);
    seconds -= hours * 3600;

    int minutes = floor(seconds / 60);
    seconds -= minutes * 60;

    // std::string result;

    char buf[12];
    if (hours == 0) {
        snprintf(buf, sizeof(buf), "%i:%02i", minutes, (int)seconds);
    } else {
        snprintf(buf, sizeof(buf), "%i:%02i:%02i", hours, minutes, (int)seconds);
    }

    return std::string(buf);
}

void audio_manager::play() {
    if (!isPlaying) {
        timer_start = SDL_GetPerformanceCounter();
        SDL_PauseAudio(0);
        isPlaying = true;
    }
}

void audio_manager::pause() {
    if (isPlaying) {
        update_offset();
        SDL_PauseAudio(1);
        isPlaying = false;
    }
}

void audio_manager::toggle_playback() {
    if (isPlaying) {
        pause();
    } else {
        play();
    }
}

void audio_manager::back() {
    pause();

    Uint64 delta_sample   = 15 * sample_rate;
    Uint64 current_sample = get_current_sample();
    Uint64 new_sample;

    if (delta_sample > current_sample) {
        new_sample = 0;
    } else {
        new_sample = current_sample - delta_sample;
    }

    timer_offset    = new_sample * SDL_GetPerformanceFrequency() / sample_rate;
    callback_offset = new_sample * 8;

    play();
}

void audio_manager::forward() {
    pause();

    Uint64 delta_sample   = 15 * sample_rate;
    Uint64 current_sample = get_current_sample();
    Uint64 new_sample;

    if ((current_sample + delta_sample) > num_samples) {
        new_sample = num_samples - sample_rate;
    } else {
        new_sample = current_sample + delta_sample;
    }

    timer_offset    = new_sample * SDL_GetPerformanceFrequency() / sample_rate;
    callback_offset = new_sample * 8;

    play();
}

void audio_manager::print() {
    if (!get_title().empty()) {
        printf("Playing \"%s\"", get_title().c_str());
        if (!get_artist().empty()) {
            printf(" by \"%s\"", get_artist().c_str());
        }
        if (!get_album().empty()) {
            printf(" on \"%s\"", get_album().c_str());
        }
        if (!get_year().empty()) {
            printf(" (%s)", get_year().c_str());
        }
    }
    printf("\n");
}
