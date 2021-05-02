#include "audio_player.h"

#include <SDL2/SDL.h>  // Audio playback

#include <algorithm>  // min, max

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

AudioPlayer::AudioPlayer(const AudioSource &source) : source_(source) {
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = source_.sample_rate();
    want.format = AUDIO_F32;
    want.channels = source_.num_channels();
    want.samples = 8192;
    want.callback = callback;
    want.userdata = (void *)this;

    if (SDL_OpenAudio(&want, &have) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        throw "FAILED";
    } else {
        if (have.format != want.format) {
            SDL_Log("We didn't get Float32 audio format.");
            throw "FAILED";
        }
    }

    timer_start_ = 0;
    timer_offset_ = 0;
    callback_offset_ = 0;
    playable_ = true;
}

AudioPlayer::~AudioPlayer() { SDL_CloseAudio(); }

void AudioPlayer::callback(void *userdata, Uint8 *stream, int len) {
    SDL_memset(stream, 0, len);
    AudioPlayer *am = (AudioPlayer *)userdata;
    const Uint8 *audio_ptr = (Uint8 *)am->source_.data().data();
    SDL_MixAudioFormat(stream, (Uint8 *)audio_ptr + am->callback_offset_,
                       AUDIO_F32, len, SDL_MIX_MAXVOLUME / 2);
    am->callback_offset_ += len;
}

void AudioPlayer::update_offset() {
    if (playing_) {
        Uint64 now = SDL_GetPerformanceCounter();
        timer_offset_ += (now - timer_start_);
        timer_start_ = 0;
    }
}

Uint64 AudioPlayer::current_sample() {
    Uint64 sample =
        (Uint64)round(current_time() * (double)source_.sample_rate());
    return std::max((Uint64)1, std::min(source_.num_samples(), sample));
}

double AudioPlayer::current_time() {
    Uint64 position = timer_offset_;

    if (playing_) {
        Uint64 now = SDL_GetPerformanceCounter();
        position += (now - timer_start_);
    }

    return position / (double)SDL_GetPerformanceFrequency();
}

std::string AudioPlayer::current_time_str() {
    double seconds = current_time();

    int hours = floor(seconds / 3600);
    seconds -= hours * 3600;

    int minutes = floor(seconds / 60);
    seconds -= minutes * 60;

    char buf[12];
    if (hours == 0)
        snprintf(buf, sizeof(buf), "%i:%02i", minutes, (int)seconds);
    else
        snprintf(buf, sizeof(buf), "%i:%02i:%02i", hours, minutes,
                 (int)seconds);

    return std::string(buf);
}

void AudioPlayer::play() {
    if (!playing_) {
        timer_start_ = SDL_GetPerformanceCounter();
        SDL_PauseAudio(0);
        playing_ = true;
    }
}

void AudioPlayer::pause() {
    if (playing_) {
        update_offset();
        SDL_PauseAudio(1);
        playing_ = false;
    }
}

void AudioPlayer::toggle_playback() {
    if (playing_)
        pause();
    else
        play();
}

void AudioPlayer::back() {
    pause();

    Uint64 delta = 15 * source_.sample_rate();
    Uint64 position = current_sample();
    Uint64 new_position;

    if (delta > position)
        new_position = 0;
    else
        new_position = position - delta;

    timer_offset_ =
        new_position * SDL_GetPerformanceFrequency() / source_.sample_rate();
    callback_offset_ = new_position * 8;

    play();
}

void AudioPlayer::forward() {
    pause();

    const Uint64 delta = 15 * source_.sample_rate();
    const Uint64 position = current_sample();
    Uint64 new_position;

    if ((position + delta) > source_.num_samples())
        new_position = source_.num_samples() - source_.sample_rate();
    else
        new_position = position + delta;

    timer_offset_ =
        new_position * SDL_GetPerformanceFrequency() / source_.sample_rate();
    callback_offset_ = new_position * 8;

    play();
}
