#ifndef AUDIO_LOAD_H
#define AUDIO_LOAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <sndfile.h>

#define BUFFER_LEN              (1 << 16)

typedef struct
{
    uint32_t  num_samples;
    uint32_t  sample_rate;
    uint32_t  num_channels;
    float*    signal;

} audio_data;

/*
const char* format_duration_str (double seconds);

const char* generate_duration_str (SF_INFO *sfinfo);
*/
void load_audio (const char *filename , audio_data *data );


#ifdef __cplusplus
}
#endif

#endif /* AUDIO_LOAD_H */

