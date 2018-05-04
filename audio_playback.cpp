#include <SDL2/SDL.h>
#include "audio_playback.h"

void MyAudioCallback(void *userdata, Uint8 *stream, int len) {
    
    SDL_memset(stream, 0, len);
    
    Uint8** audio_ptr = (Uint8**) userdata;
    
    SDL_MixAudioFormat( stream, (Uint8*) *audio_ptr, AUDIO_F32, len, SDL_MIX_MAXVOLUME / 2);
    
    *audio_ptr += len;
    
}

void audio_playback(audio_data *input_data) {

    SDL_AudioSpec want, have;
    
    SDL_memset(&want, 0, sizeof(want));
    want.freq = input_data->sample_rate;
    want.format = AUDIO_F32;
    want.channels = input_data->num_channels;
    want.samples = 8192;
    want.callback = MyAudioCallback;
    want.userdata = (void*) &input_data->signal;
    
    if (SDL_OpenAudio(&want, &have) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    } else {
        if (have.format != want.format) {
            SDL_Log("We didn't get Float32 audio format.");
        }
    }
}