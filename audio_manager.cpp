#include "audio_manager.h"

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

void audio_manager::callback(void *userdata, Uint8 *stream, int len) {
    
    SDL_memset(stream, 0, len);
    
    Uint8** audio_ptr = (Uint8**) userdata;
    
    SDL_MixAudioFormat( stream, (Uint8*) *audio_ptr, AUDIO_F32, len, SDL_MIX_MAXVOLUME / 2);
    
    *audio_ptr += len;
    
}

void audio_manager::setup_playback() {

    SDL_AudioSpec want, have;
    
    SDL_memset(&want, 0, sizeof(want));
    want.freq = this->sample_rate;
    want.format = AUDIO_F32;
    want.channels = this->num_channels;
    want.samples = 8192;
    want.callback = callback;
    want.userdata = (void*) &this->data;
    
    if (SDL_OpenAudio(&want, &have) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    } else {
        if (have.format != want.format) {
            SDL_Log("We didn't get Float32 audio format.");
        }
    }
    
    this->isPlayable = true;
    
}

void audio_manager::play() {
    SDL_PauseAudio(0);
}

void audio_manager::pause() {
    SDL_PauseAudio(1);
}