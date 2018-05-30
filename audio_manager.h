#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <sndfile.h>    // Audio file loading
#include <SDL2/SDL.h>   // Audio playback

#define BUFFER_LEN              (1 << 16)

class audio_manager {
public:
    audio_manager();
    audio_manager(const char *filename);
    audio_manager(const audio_manager& orig);
    virtual ~audio_manager();
    
    void play();
    void pause();
    void toggle_playback();
    void back();
    void forward();
    
    bool is_loaded() {return isLoaded;};
    bool is_playable() {return isPlayable;};
    bool is_playing() {return isPlaying;};
    Uint64 get_num_channels() {return num_channels;};
    Uint64 get_num_samples() {return num_samples;};
    Uint64 get_sample_rate() {return sample_rate;};
    Uint64 get_current_sample();
    double get_current_time();
    float* get_data_ptr() {return data;};
    
private:
    bool isLoaded = false;
    bool isPlayable = false;
    bool isPlaying = false;
    
    double current_time = 0.0;
    
    Uint64 timer_start = 0;
    Uint64 timer_offset = 0;
    Uint64 callback_offset = 0;
    
    std::string filename;
    Uint64 num_channels = 0;
    Uint64 num_samples = 0;
    Uint64 sample_rate = 0;
    float *data = NULL;
    
    void load_file();
    void setup_playback();
    
    static void callback(void *userdata, Uint8 *stream, int len);
    void update_offset();
    
};

#endif /* AUDIO_MANAGER_H */

