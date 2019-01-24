#include <cstdlib>
#include <exception>
#include <functional>

#include <vector>
#include <iostream>
#include <memory>

#include "audio_manager.h"
#include "framebuffer.h"
#include "shader_program.h"
#include "stft.h"
#include "window.h"

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
    
    if (!audio.is_playable() || audio.get_num_samples()==0) {
        fprintf(stderr,"Audio problem, bailing out!\n"); fflush(stderr);
        return 1;
    }
    
    // Print metadata
    if (!audio.get_title().empty()) {
        printf("Playing \"%s\"",audio.get_title().c_str());
        if (!audio.get_artist().empty()) {
            printf(" by \"%s\"",audio.get_artist().c_str());
        }
        if (!audio.get_album().empty()) {
            printf(" on \"%s\"",audio.get_album().c_str());
        }
        if (!audio.get_year().empty()) {
            printf(" (%s)",audio.get_year().c_str());
        }
    }
    printf("\n");
            
    const int num_channels = audio.get_num_channels();
    
    // Configure spectrogram transform
    SpectrogramConfig config;
    config.padding_mode = PAD;
    config.window_length = 4096;
    config.window_overlap = config.window_length * .5;
    config.transform_length = config.window_length * 4;
    config.window_type = HAMMING;
    
    // Create STFT object
    STFT stft(audio, config, 4096*2);
    
    // Analyze for maximum power
    stft.analyze();
    
    // Get number of frequencies
    const unsigned long num_frequencies = stft.maxGoodFreq();
    const unsigned long num_trapezoids = num_frequencies - 1;
    const unsigned long num_triangles = 2 * num_trapezoids;
    const unsigned long num_vertices = 3 * num_triangles;
    
    // Allocate vertex vector
    std::vector<std::vector<float>> vertices(num_channels);
    for (int i=0; i<num_channels; i++) {
        vertices[i] = std::vector<float>(num_vertices);
    }
        
    // Initialize window and context
    Window wnd;
    
    // Create a framebuffer
    FrameBuffer fb(&wnd);
    
    // Setup shaders
    ShaderProgram main_shader(vertex_source, fragment_source);
    
    main_shader.use();
    
    // Set static uniforms
    main_shader.set_uniform("num_freq",(int)num_frequencies);
    main_shader.set_uniform("resolution",(float) fb.width(), fb.height());
    
    // Set up vertex buffer/array object for each channel
    GLuint VBO[num_channels];
    GLuint VAO[num_channels];
    glGenVertexArrays(num_channels, VAO );
    
    for (int channel=0; channel<num_channels; channel++) {
        
        glGenBuffers(1, &VBO[channel]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[channel]);
        glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
        
        glBindVertexArray( VAO[channel] );
        
        main_shader.set_attrib("amplitude",sizeof(float));
        
    }
    
    // Enable v-sync
    SDL_GL_SetSwapInterval(1);
    
    // Disable depth, enable alpha blending
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(false);
    
    // Check for GL errors
    wnd.check_errors();
    
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
                        main_shader.use();
                        main_shader.set_uniform("resolution",(float) fb.width(), fb.height());
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
            
            // Copy amplitude values for each trapzeoid/triangle
            for (unsigned long trap=0; trap<num_trapezoids; trap++) {
                        
                vertices[channel][6*trap+1] = power[trap];
                vertices[channel][6*trap+2] = power[trap+1];                
                vertices[channel][6*trap+3] = power[trap+1];
                
            }
            
            // Update the buffer
            glBindBuffer(GL_ARRAY_BUFFER, VBO[channel]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, num_vertices * sizeof(GLfloat), vertices[channel].data() );
            glBindVertexArray( VAO[channel] );

            if (channel==0) {
                main_shader.set_uniform("multiplier", -1.0f);
            } else {
                main_shader.set_uniform("multiplier", 1.0f);
            }
            
            glDrawArrays(GL_TRIANGLES, 0, num_vertices );

        }
     
        fb.unbind();
        fb.draw();
        wnd.swap();
    
    };
    
    // Cleanup
    audio.pause();
    
    return 0;
}

