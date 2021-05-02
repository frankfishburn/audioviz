#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <string>
#include <vector>

#include "audio_source.h"

class AudioPlayer {
   public:
    AudioPlayer(AudioSource& source);
    ~AudioPlayer();

    // Playback controls
    void play();
    void pause();
    void toggle_playback();
    void back();
    void forward();

    // Query player state
    bool          playable() const { return playable_; };
    bool          playing() const { return playing_; };
    unsigned long current_sample();
    double        current_time();
    std::string   current_time_str();

   private:
    // The audio source
    AudioSource& source_;

    // Player state
    bool playable_ = false;
    bool playing_  = false;

    // Player timing data
    unsigned long timer_start_;
    unsigned long timer_offset_;
    unsigned long callback_offset_;

    // Initialization
    void        setup_playback();
    static void callback(void* userdata, uint8_t* stream, int len);
    void        update_offset();
};

#endif /* AUDIO_PLAYER_H */
