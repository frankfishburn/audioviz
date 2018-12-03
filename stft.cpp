#include <cmath> // INFINITY
#include <algorithm> // min,max
#include <cstdio> // printf

#include "stft.h"

STFT::STFT(audio_manager& audio, SpectrogramConfig& inputconfig, unsigned long input_samples) {
    
    num_channels = audio.get_num_channels();
    num_samples = audio.get_num_samples();
    audio_ptr = audio.get_data();
            
    // Configure input data properties
    props.data_size = sizeof(float);
    props.sample_rate = audio.get_sample_rate();
    props.num_samples = input_samples;
    props.stride = num_channels;
    
    // Configure spectrogram transform
    config.padding_mode = inputconfig.padding_mode;
    config.window_length = inputconfig.window_length;
    config.window_overlap = inputconfig.window_overlap;
    config.transform_length = inputconfig.transform_length;
    config.window_type = inputconfig.window_type;
    
    // Create spectrogram object
    program = spectrogram_create( &props, &config );
    
    // Get derived output dimensions
    time_len = 1; //spectrogram_get_timelen( program );
    freq_len = spectrogram_get_freqlen( program );
    
    // Get frequency vector
    freq.resize(freq_len);
    spectrogram_get_freq(program, (void*) freq.data());
    
    // Allocate spectrogram power for each channel
    power.resize(num_channels);
    for (int i=0; i<num_channels; i++) {
        power[i].resize(time_len*freq_len);
        std::fill(power[i].begin(), power[i].end(), 0);
    }
    
}

STFT::STFT(const STFT& orig) {
}

STFT::~STFT() {

spectrogram_destroy(program);

}

void STFT::analyze() {
    
    maxpower = 1.0f;
    float tmpmaxpower = -INFINITY;
    
    for (double pos=0.0; pos<1.0; pos+=.01) {
        
        long start_index = (long) round(pos * num_samples);
        
        for (int channel=0; channel<num_channels; channel++) {
            
            // Compute spectrogram
            compute(channel,start_index);
            
            // Get maximum power
            for (unsigned long time=0; time<time_len; time++)
                for (unsigned long freq=0; freq<freq_len; freq++)
                    tmpmaxpower = std::max( tmpmaxpower , power[channel][time*freq_len + freq] );
            
        }
    }
    
    maxpower = tmpmaxpower;

}

void STFT::compute(const int channel, const long sample_index) {
    
    const long max_sample_index = num_samples - props.num_samples - 1;
    
    long start_index = sample_index - props.num_samples/2;
    start_index = std::max( start_index , (long)0 );
    start_index = std::min( start_index , max_sample_index );
    
    spectrogram_execute(program, (void*) (audio_ptr + num_channels * start_index + channel) );
    spectrogram_get_power_periodogram(program, (void*) power[channel].data() );
    
    // Rescale based on frequency and log transform
    for (unsigned long time=0; time<time_len; time++) {
        for (unsigned long freq=0; freq<freq_len; freq++) {
            float scale = (float) std::max((unsigned long)100,freq);
            power[channel][time*freq_len + freq] = log( 1.0f + power[channel][time*freq_len + freq] * scale * scale ) / maxpower;
        }
    }
    
}
