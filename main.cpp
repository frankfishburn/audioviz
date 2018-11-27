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
    float* audio_ptr = audio.get_data();
            
    // Create spectrogram object
    SpectrogramInput props;
    props.data_size = sizeof(float);
    props.sample_rate = sample_rate;
    props.num_samples = 8192;
    props.stride = num_channels;
    
    SpectrogramConfig config;
    config.padding_mode = PAD;
    config.window_length = 8192;
    config.window_overlap = config.window_length * .9;
    config.transform_length = 8192*2;
    config.window_type = HAMMING;
    
    SpectrogramTransform *mySTFT = spectrogram_create( &props, &config );
    
    unsigned long freq_len = spectrogram_get_freqlen( mySTFT );
    
    // Allocate STFT frequency vector
    float *freq = (float*) calloc(freq_len, sizeof(float));
    spectrogram_get_freq(mySTFT, (void*) freq);
    
    // Allocate spectrogram power for each channel
    float *power[num_channels];
    float *power6[num_channels];
    for (int i=0; i<num_channels; i++) {
        power[i] = (float*) calloc(freq_len, sizeof(float));
        power6[i] = (float*) calloc(6 * freq_len, sizeof(float));
    }
    
    // Get maximum power at each frequency
    float maxpower = -INFINITY;
    for (double pos=0.0; pos<1.0; pos+=.01) {
        
        unsigned long start_index = min((unsigned long) round(pos * num_samples),num_samples-props.num_samples-1);
        
        for (int channel=0; channel<num_channels; channel++) {
            
            spectrogram_execute(mySTFT, (void*) (audio_ptr + num_channels * start_index + channel) );
            spectrogram_get_power(mySTFT, (void*) power[channel]);
                        
            for (unsigned long freq=0; freq<freq_len; freq++)
                maxpower = max( maxpower , log( 1.0f + power[channel][freq] * max((unsigned long)100,freq) * max((unsigned long)100,freq) ) );
            
        }
    }
   
    // Initialize window and context
    SDL_Window* wnd = init_GL();
    
    // Setup shaders
    ShaderProgram main_shader(vertex_source, fragment_source);
    
    main_shader.use();
    
    // Set static uniforms
    int freq_draw_len = freq_len / 5;
    main_shader.set_uniform("num_freq",freq_draw_len);
    
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    main_shader.set_uniform("resolution",(float)viewport[2],(float)viewport[3]);
    
    // Set up vertex buffer/array object for each channel
    GLuint VBO[num_channels];
    GLuint VAO[num_channels];
    glGenVertexArrays(num_channels, VAO );
    
    for (int channel=0; channel<num_channels; channel++) {
        
        glGenBuffers(1, &VBO[channel]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[channel]);
        glBufferData(GL_ARRAY_BUFFER, 6 * freq_draw_len * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
        
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
    bool quit=false;
    while (!quit) {
        SDL_Event e;
        
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT:
                    quit=true;
                    break;
                    
                case SDL_WINDOWEVENT:
                    if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        fb.freshen();
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                        glViewport(0, 0, e.window.data1, e.window.data2);
                        glGetIntegerv(GL_VIEWPORT,viewport);
                        main_shader.use();
                        main_shader.set_uniform("resolution",(float)viewport[2],(float)viewport[3]);
                    }
                    break;
                
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_SPACE: audio.toggle_playback(); break;
                        case SDLK_LEFT: audio.back(); break;
                        case SDLK_RIGHT: audio.forward(); break;
                        case SDLK_ESCAPE: quit=true; break;
                    }
                    break;
                 
            }
        }

        fb.bind();
        main_shader.use();
        
        // Update current time
        long current_sample = audio.get_current_sample();
        long start_index = max( (long) 0 , current_sample - (long) (props.num_samples + config.window_length) );
        
        // Render each channel
        for (int channel=0; channel<num_channels; channel++){
        
            // Compute STFT
            spectrogram_execute(mySTFT, (void*) (audio_ptr + num_channels * start_index + channel) );
            spectrogram_get_power(mySTFT, (void*) power[channel]);
            
            // Rescale power
            for (unsigned long freq=0; freq<freq_len; freq++)
                power[channel][freq] = log(1.0 + power[channel][freq] * max((unsigned long)100,freq) * max((unsigned long)100,freq) ) / maxpower;
                    
            // Copy values into triangle format
            for (unsigned long freq=0; freq<freq_len; freq++){
                unsigned long baseidx = freq*6;
                power6[channel][baseidx+1] = power[channel][freq];
                power6[channel][baseidx+2] = power[channel][freq];
                power6[channel][baseidx+4] = power[channel][freq];
            }
            
            // Update the buffer
            glBindBuffer(GL_ARRAY_BUFFER, VBO[channel]);    
            glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * freq_draw_len * sizeof(GLfloat), power6[channel]);
            glBindVertexArray( VAO[channel] );

            if (channel==0) {
                main_shader.set_uniform("multiplier", -1.0f);
            } else {
                main_shader.set_uniform("multiplier", 1.0f);
            }
            
            glDrawArrays(GL_TRIANGLES, 0, 6*freq_draw_len );

        }
     
        fb.unbind();
        fb.draw();

        SDL_GL_SwapWindow(wnd);
        
    };

    // Cleanup
    spectrogram_destroy(mySTFT);
    deinit_GL();
    
    return 0;
}

