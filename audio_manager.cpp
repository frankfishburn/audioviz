#include "audio_manager.h"
#include <algorithm> // min, max

audio_manager::audio_manager() {

}

audio_manager::audio_manager(const char *filename) {
    load_file(filename);
    setup_playback();
}

audio_manager::audio_manager(const audio_manager& orig) {
}

audio_manager::~audio_manager() {
    
    if (isLoaded) {
        free(data);
        isLoaded=false;
        isPlayable=false;
    }
    
}

void audio_manager::load_file(const char *filename) {

    char strbuffer[BUFFER_LEN];
    double audiodata[BUFFER_LEN];
    SNDFILE *file;
    SF_INFO sfinfo;

    memset (&sfinfo, 0, sizeof(sfinfo));

    file = sf_open(filename, SFM_READ, &sfinfo);
    
    // Error stuff
    if (file == NULL)
    {
        fprintf(stderr,"Error : Not able to open input file %s.\n", filename);
        fflush(stderr);
        memset(audiodata, 0, sizeof (audiodata));
        sf_command(file, SFC_GET_LOG_INFO, strbuffer, BUFFER_LEN);
        puts(strbuffer);
        puts(sf_strerror (NULL));
        return;
    }

    // Save parameters
    num_channels = (long) sfinfo.channels;
    sample_rate  = (long) sfinfo.samplerate;
    num_samples  = sfinfo.frames;
    
    // Allocate audio data arrays
    data = (float*) malloc( sizeof(float) * num_channels * num_samples );

    // Read audio data to buffer
    sf_readf_float(file, data , num_samples * num_channels );

    // Close audio file
    sf_close(file);
    
    this->isLoaded = true;

}

void audio_manager::setup_playback() {

    SDL_AudioSpec want, have;
    
    SDL_memset(&want, 0, sizeof(want));
    want.freq = this->sample_rate;
    want.format = AUDIO_F32;
    want.channels = this->num_channels;
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
    
    this->isPlayable = true;
    
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
