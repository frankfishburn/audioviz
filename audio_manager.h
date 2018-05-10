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
    
    void load_file(const char *filename);
    void setup_playback();
    void play();
    void pause();
    
    bool is_loaded() {return isLoaded;};
    bool is_playable() {return isPlayable;};
    int get_num_channels() {return num_channels;};
    long get_num_samples() {return num_samples;};
    long get_sample_rate() {return sample_rate;};
    float* get_data_ptr() {return data;};
    
private:
    bool isLoaded = false;
    bool isPlayable = false;
    
    int num_channels = 0;
    int num_samples = 0;
    int sample_rate = 0;
    float *data;
    std::string filename;
    
    static void callback(void *userdata, Uint8 *stream, int len);
    
};

#endif /* AUDIO_MANAGER_H */

