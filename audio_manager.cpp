#include "audio_manager.h"

#include <algorithm> // min, max
#include <SDL2/SDL.h>   // Audio playback

extern "C" { 
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <vector>
}

audio_manager::audio_manager(const char *filename) {

    input_file = std::string(filename);;
    load_file();
    setup_playback();

}

audio_manager::audio_manager(const audio_manager& orig) {

    data.resize(orig.num_samples * orig.num_channels);
    memcpy(data.data(), orig.data.data(), num_samples * num_channels * sizeof(float));
    
}

audio_manager::~audio_manager() {
    
    if (isLoaded) {
        isLoaded=false;
        isPlayable=false;
    }
    
}

void audio_manager::load_file() {
    
    const char* filename = input_file.c_str();
    
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    
    if (avformat_open_input(&format, filename, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", filename); fflush(stderr);
        return;
    }
    printf("Format %s, duration %ld us\n", format->iformat->long_name, format->duration);
    
    if (avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", filename); fflush(stderr);
        return;
    }
    
    // Find the index of the first audio stream
    AVStream* stream = format->streams[0];
    for (unsigned int i=0; i<format->nb_streams; i++) {
        stream = format->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            break;
    }
    if (stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
        fprintf(stderr, "Could not retrieve audio stream from file '%s'\n", filename); fflush(stderr);
        return;
    }
    
    // Setup decoder    
    AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    AVCodecContext* context = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(context, stream->codecpar);
    
    if (avcodec_open2(context, codec, NULL) < 0) {
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n", stream->id, filename); fflush(stderr);
        return;
    }
    
    printf("Audio Codec: %d channels, sample rate %d\n", stream->codecpar->channels, stream->codecpar->sample_rate);
    
    num_channels = stream->codecpar->channels;
    sample_rate = stream->codecpar->sample_rate;
    
    // prepare to read data
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* outframe = av_frame_alloc();
    av_frame_copy_props(outframe, frame);
    
    outframe->channel_layout = AV_CH_LAYOUT_STEREO;
    outframe->sample_rate = stream->codecpar->sample_rate;
    outframe->format = AV_SAMPLE_FMT_FLT;
    
    av_init_packet(packet);
    
    if (!frame) {
        fprintf(stderr, "Error allocating the frame\n");
        return;
    }
    
    // prepare resampler
    struct SwrContext* swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count",  stream->codecpar->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 2, 0);
    av_opt_set_int(swr, "in_channel_layout",  stream->codecpar->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr, "in_sample_rate", stream->codecpar->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", stream->codecpar->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", context->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT,  0);
    swr_init(swr);
    if (!swr_is_initialized(swr)) {
        fprintf(stderr, "Resampler has not been properly initialized\n");
        return;
    }

    // iterate through frames
    unsigned long oldsize = 0;
    unsigned long newsize = 0;
    while ( av_read_frame(format,packet) >=0 ) {
        
        if (avcodec_send_packet(context, packet) < 0) {
            printf("Bad packet at size=%lu\n",newsize);
            break;
        }
        if (avcodec_receive_frame(context, frame) < 0 ) {
            printf("Bad frame at size=%lu\n",newsize);
            break;
        }
                
        swr_convert_frame(swr,outframe,frame);
        
        oldsize = newsize;
        newsize = oldsize + frame->nb_samples * frame->channels;
        
        // Expand array and copy frame contents
        data.resize(newsize);
        memcpy(data.data()+oldsize, outframe->data[0], outframe->nb_samples * outframe->channels * sizeof(float) );

    }

    num_samples = newsize / num_channels;
    
    // clean up
    swr_close(swr);
    swr_free(&swr);
    av_packet_free(&packet);
    av_frame_free(&frame);
    av_frame_free(&outframe);
    avcodec_close(context);
    avformat_free_context(format);
    
    printf("Loaded file: %lu channels, %lu samples, %lu Hz\n",num_channels,num_samples,sample_rate); fflush(stdout);
    
    isLoaded = true;
}

void audio_manager::setup_playback() {
    
    if (!isLoaded)
        return;
    
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = sample_rate;
    want.format = AUDIO_F32;
    want.channels = num_channels;
    want.samples = 8192;
    want.callback = callback;
    want.userdata = (void*) this;
    
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

void audio_manager::callback(void *userdata, Uint8 *stream, int len) {
    
    SDL_memset(stream, 0, len);
    audio_manager *am = (audio_manager*) userdata;
    Uint8** audio_ptr = (Uint8**) &(am->data);
    SDL_MixAudioFormat( stream, (Uint8*) *audio_ptr + am->callback_offset , AUDIO_F32, len, SDL_MIX_MAXVOLUME / 2);
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
    
    Uint64 current_sample = (Uint64) round( get_current_time() * (double) sample_rate );
    current_sample = std::min( num_samples , current_sample );
    current_sample = std::max( (Uint64) 1 , current_sample );
    return current_sample;
    
}

double audio_manager::get_current_time() {
    
    Uint64 position = timer_offset;
    
    if (isPlaying) {
        Uint64 now = SDL_GetPerformanceCounter();
        position += (now - timer_start);
    }
    
    return position / (double) SDL_GetPerformanceFrequency();
    
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
    
    Uint64 delta_sample = 15 * sample_rate;
    Uint64 current_sample = get_current_sample();
    Uint64 new_sample;
    
    if (delta_sample > current_sample) {
        new_sample = 0;
    } else {
        new_sample = current_sample - delta_sample;
    }
    
    timer_offset = new_sample * SDL_GetPerformanceFrequency() / sample_rate;
    callback_offset = new_sample * 8;
    
    play();
    
}

void audio_manager::forward() {
    
    pause();
    
    Uint64 delta_sample = 15 * sample_rate;
    Uint64 current_sample = get_current_sample();
    Uint64 new_sample;
    
    if ((current_sample + delta_sample) > num_samples) {
        new_sample = num_samples-sample_rate;
    } else {
        new_sample = current_sample + delta_sample;
    }
    
    timer_offset = new_sample * SDL_GetPerformanceFrequency() / sample_rate;
    callback_offset = new_sample * 8;
    
    play();
    
}
