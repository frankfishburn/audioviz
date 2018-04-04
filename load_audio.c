#include "load_audio.h"

void load_audio (const char *filename , audio_data *data )
{

    char    strbuffer [BUFFER_LEN] ;
    double   audiodata [BUFFER_LEN] ;
    SNDFILE         *file ;
    SF_INFO         sfinfo ;

    memset (&sfinfo, 0, sizeof (sfinfo)) ;

    // Error stuff
    if ((file = sf_open (filename, SFM_READ, &sfinfo)) == NULL)
    {
        printf("Error : Not able to open input file %s.\n", filename);
        fflush(stdout);
        memset(audiodata, 0, sizeof (audiodata));
        sf_command(file, SFC_GET_LOG_INFO, strbuffer, BUFFER_LEN);
        puts(strbuffer);
        puts(sf_strerror (NULL));
        return;
    }

    // Save important info
    data->sample_rate  = (unsigned int) sfinfo.samplerate;
    data->num_channels = (unsigned int) sfinfo.channels;
    //data->duration     = QString::fromLocal8Bit(generate_duration_str(&sfinfo));
    data->num_samples  = sfinfo.frames;

    // Allocate audio data arrays
    float *tmpbuffer, *audiobuffer;
    tmpbuffer      = (float*) malloc( sizeof(float) * data->num_channels * data->num_samples );
    audiobuffer    = (float*) malloc( sizeof(float) * data->num_channels * data->num_samples );
    data->signal = audiobuffer;

    // Read audio data to buffer
    sf_readf_float(file, tmpbuffer , data->num_samples * data->num_channels );

    // Close audio file
    sf_close(file);

    // Deinterleave so channels are one-after-another
    unsigned long samp; unsigned int chan;
    for (samp = 0; samp<data->num_samples; samp++)
    {
        for (chan = 0; chan<data->num_channels; chan++)
        {
            *(audiobuffer + samp + (data->num_samples*chan) ) =  *(tmpbuffer + (samp * data->num_channels + chan));
        }
    }

    // Deallocate temporary buffer
    free( tmpbuffer );

    printf("Loaded file. %u channels @ %u Hz for %u samples\n",data->num_channels,data->sample_rate,data->num_samples);
    
    return;
}
