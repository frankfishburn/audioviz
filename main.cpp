#include <cstdlib>
#include <exception>
#include <functional>

#include <vector>
#include <iostream>
#include <memory>

#include "spectrogram.h"

#include "audio_manager.h"

#include "opengl.h"
#include "shader_program.h"
#include "framebuffer.h"

const char *vertex_source = 
#include "shaders/vert_direct_freq.glsl"
;
const char *fragment_source = 
#include "shaders/frag_plain.glsl"
;

using namespace std;

std::function<void()> loop;
void main_loop() { loop(); }

int main(int argc, char** argv) {
    
    if (argc<2) {
        fprintf(stderr,"Please supply a filename to a music file.\n");
        return EXIT_FAILURE;
    }
    
    // Load audio and setup playback
    audio_manager audio(argv[1]);
   
    if (!audio.is_playable()) {
        fprintf(stderr,"Audio problem, bailing out!\n");
        return 1;
    }
    
    const int sample_rate = audio.get_sample_rate();
    const int num_samples = audio.get_num_samples();
    const int num_channels = audio.get_num_channels();
        
    // Create spectrogram object
    InputProps props;
    props.data_size = sizeof(float);
    props.sample_rate = sample_rate;
    props.num_samples = 8192;
    props.stride = num_channels;
    
    StftConfig config;
    config.padding_mode = TRUNCATE;
    config.window_length = 8192;
    config.window_overlap = 4096;
    config.window_type = HAMMING;
    
    SpectrogramTransform *mySTFT = spectrogram_create( &props, &config );
    
    unsigned long freq_len = spectrogram_get_freqlen( mySTFT );
    
    // Allocate STFT frequency vector
    float *freq = (float*) calloc(freq_len, sizeof(float));
    spectrogram_get_freq(mySTFT, (void*) freq);
    
    // Allocate spectrogram power for each channel
    float *power[num_channels];
    for (int i=0; i<num_channels; i++) {
        power[i] = (float*) calloc(freq_len, sizeof(float));  
    }
    
    // Get maximum power at each frequency
    float *maxpower = (float*) calloc(freq_len, sizeof(float));
    for (unsigned int start_index=0; start_index<num_samples-props.num_samples; start_index+=1470) {
        for (int channel=0; channel<num_channels; channel++) {
            
            spectrogram_execute(mySTFT, (void*) (audio.get_data() + num_channels * start_index + channel) );
            spectrogram_get_power_periodogram(mySTFT, (void*) power[channel]);
            
            for (unsigned long freq=0; freq<freq_len; freq++)
                maxpower[freq] = max( maxpower[freq], power[channel][freq] );
            
        }
    }
    
    // Initialize window and context
    SDL_Window* wnd = init_GL();
    
    // Setup shaders
    ShaderProgram main_shader(vertex_source, fragment_source);
    
    main_shader.use();
    
    // Set static uniforms
    int freq_draw_len = freq_len / 20;
    main_shader.set_uniform("num_freq",freq_draw_len);
    
    // Set up vertex buffer/array object for each channel
    GLuint VBO[num_channels];
    GLuint VAO[num_channels];
    glGenVertexArrays(num_channels, VAO );
    
    for (int channel=0; channel<num_channels; channel++) {
        
        glGenBuffers(1, &VBO[channel]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[channel]);
        glBufferData(GL_ARRAY_BUFFER, freq_len * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
        
        glBindVertexArray( VAO[channel] );
        
        main_shader.set_attrib("amplitude",sizeof(float));
        
    }
    
    // Create a framebuffer
    FrameBuffer fb(wnd);
    
    // Enable v-sync
    SDL_GL_SetSwapInterval(1);
    
    // Disable depth, enable alpha blending
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(false);
    
    // Check for GL errors
    glPrintErrors();
    
    // Start audio
    audio.play();
    
    // Render Loop
    loop = [&]
    {
        SDL_Event e;
        
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT:
                    std::abort();
                    break;
                    
                case SDL_WINDOWEVENT:
                    if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        fb.freshen();
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                        glViewport(0, 0, e.window.data1, e.window.data2);
                    }
                    break;
                
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_SPACE: audio.toggle_playback(); break;
                        case SDLK_LEFT: audio.back(); break;
                        case SDLK_RIGHT: audio.forward(); break;
                    }
                    break;
                 
            }
        }

        fb.bind();
        main_shader.use();
        
        // Update current time
        long current_sample = audio.get_current_sample();
        long start_index = max( (long) 0 , current_sample - (long) props.num_samples/2 );
        
        // Render each channel
        for (int channel=0; channel<num_channels; channel++){
        
            // Compute STFT
            spectrogram_execute(mySTFT, (void*) (audio.get_data() + num_channels * start_index + channel) );
            spectrogram_get_power_periodogram(mySTFT, (void*) power[channel]);
            
            // Rescale power
            for (unsigned long freq=0; freq<freq_len; freq++) {
                power[channel][freq] /= maxpower[freq];
            }

            // Update the buffer
            glBindBuffer(GL_ARRAY_BUFFER, VBO[channel]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, freq_len * sizeof(GLfloat), power[channel]);
            
            glBindVertexArray( VAO[channel] );
            
            if (channel==0){
                main_shader.set_uniform("RGB", 1.0f, 0.0f, 0.0f);
            } else {
                main_shader.set_uniform("RGB", 0.0f, 0.0f, 1.0f);                
            }
            
            main_shader.set_uniform("multiplier", 1.0f);
            glDrawArrays(GL_LINE_STRIP, 0, freq_draw_len );
            
            main_shader.set_uniform("multiplier", -1.0f);
            glDrawArrays(GL_LINE_STRIP, 0, freq_draw_len );
        }
     
        fb.unbind();
        fb.draw();

        SDL_GL_SwapWindow(wnd);
        
    };

    while(true) main_loop();
    
    return 0;
}

