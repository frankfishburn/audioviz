#include <cstdlib>
#include <exception>
#include <functional>

#include <vector>
#include <iostream>
#include <memory>

#include "audio_manager.h"

#include "opengl.h"
#include "shader_program.h"
#include "framebuffer.h"
#include "stft.h"

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
    
    const int num_channels = audio.get_num_channels();
    
    // Configure spectrogram transform
    SpectrogramConfig config;
    config.padding_mode = PAD;
    config.window_length = 128;
    config.window_overlap = config.window_length * .5;
    config.transform_length = config.window_length * 2;
    config.window_type = HAMMING;
    
    // Create STFT object
    STFT stft(audio, config, 8192);
    
    // Analyze for maximum power
    stft.analyze();
    
    // Get number of frequencies
    unsigned long freq_len = stft.numFreq();
    int freq_draw_len = freq_len / 3;
    
    // Create the power vectors for rendering triangles
    std::vector<std::vector<float>> powertri;
    powertri.resize(num_channels);
    for (int i=0; i<num_channels; i++) {
        powertri[i].resize(6*freq_len);
        std::fill( powertri[i].begin(), powertri[i].end(), 0);
    }
    
    // Initialize window and context
    SDL_Window* wnd = init_GL();
    
    // Setup shaders
    ShaderProgram main_shader(vertex_source, fragment_source);
    
    main_shader.use();
    
    // Set static uniforms
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
        
        // Render each channel
        for (int channel=0; channel<num_channels; channel++){
        
            stft.compute( channel, current_sample );
            float* power = stft.getPowerPtr( channel );
            
            // Copy values into triangle format
            for (unsigned long freq=0; freq<freq_len; freq++){
                unsigned long baseidx = freq*6;
                powertri[channel][baseidx+1] = power[freq];
                powertri[channel][baseidx+2] = power[freq];
                powertri[channel][baseidx+4] = power[freq];
            }
            
            // Update the buffer
            glBindBuffer(GL_ARRAY_BUFFER, VBO[channel]);    
            glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * freq_draw_len * sizeof(GLfloat), powertri[channel].data() );
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
    deinit_GL();
    
    return 0;
}

