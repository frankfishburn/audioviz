#ifndef AUDIO_PLAYBACK_H
#define AUDIO_PLAYBACK_H

#include "load_audio.h"

void MyAudioCallback(void *userdata, Uint8 *stream, int len);
void audio_playback(audio_data *input_data);

#endif /* AUDIO_PLAYBACK_H */

