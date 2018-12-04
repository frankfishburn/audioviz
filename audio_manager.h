#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <vector>
#include <string>

class audio_manager {
public:

    audio_manager(const char *filename);
    audio_manager(const audio_manager& orig);
    virtual ~audio_manager();

    // Playback controls
    void play();
    void pause();
    void toggle_playback();
    void back();
    void forward();
    
    // Query player state
    bool is_loaded() {return isLoaded;};
    bool is_playable() {return isPlayable;};
    bool is_playing() {return isPlaying;};
    unsigned long get_current_sample();
    double get_current_time();
    
    // Query audio file properties
    unsigned long get_num_channels() {return num_channels;};
    unsigned long get_num_samples() {return num_samples;};
    unsigned long get_sample_rate() {return sample_rate;};
    float* get_data() {return data.data();};
    
private:
    
    // Player state
    bool isLoaded = false;
    bool isPlayable = false;
    bool isPlaying = false;
    
    // Player timing data
    unsigned long timer_start = 0;
    unsigned long timer_offset = 0;
    unsigned long callback_offset = 0;
    
    // Audio file properties
    unsigned long num_channels = 0;
    unsigned long num_samples = 0;
    unsigned long sample_rate = 0;
    std::vector<float> data;
    std::string input_file;
    
    // Initialization
    void load_file(); // File
    void load_file2(); // File
    void setup_playback(); // Player
    static void callback(void *userdata, uint8_t *stream, int len);
    void update_offset();
    
};

#endif /* AUDIO_MANAGER_H */

