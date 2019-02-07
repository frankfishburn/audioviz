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

const char *src_render_vert = 
#include "shaders/render_vert.glsl"
;
const char *src_render_frag = 
#include "shaders/render_frag.glsl"
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
    
    audio.print();
            
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
    const unsigned long num_vertices = 4 * num_frequencies;
    
    // Allocate vertex vector
    std::vector<float> vertices(num_vertices);
        
    // Initialize window and context
    Window wnd;
    
    // Create a framebuffer
    FrameBuffer fb(&wnd);
    
    // Setup shaders
    ShaderProgram main_shader(src_render_vert, src_render_frag);
    
    // Set static uniforms
    main_shader.set_uniform("num_freq",(int)num_frequencies);
    main_shader.set_uniform("resolution",(float) fb.width(), fb.height());
    
    // Set up vertex buffer/array object for each channel
    GLuint VBO;
    GLuint VAO;
    glGenVertexArrays(1, &VAO );
    
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
        
    glBindVertexArray( VAO );
    main_shader.set_attrib("amplitude", 1);
    
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
    double start_time = 0;
    double current_time = 0;
    unsigned long frame_count = 0;
         
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

        float* power_left;
        float* power_right;
        
        stft.compute( 0, current_sample );
        power_left = stft.getPowerPtr(0);
        
        if (num_channels == 1) {
            // Mono
            power_right = power_left;
            
        } else {
            // Stereo
            stft.compute( 1, current_sample );
            power_right = stft.getPowerPtr(1);
        }
        
        // Copy left channel power spectrum into vertex buffer
        for (unsigned long freq=0; freq<num_frequencies; freq++) {
            vertices[2*freq+1] = -power_left[freq];
        }
        
        // Copy right channel power spectrum into vertex buffer
        for (unsigned long freq=0; freq<num_frequencies; freq++) {
            vertices[num_vertices - 2*freq] = power_right[freq];
        }
        
        // Update the buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, num_vertices * sizeof(GLfloat), vertices.data() );
        glBindVertexArray( VAO );
        glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices );
        
        fb.unbind();
        fb.draw();
        wnd.swap();
        
        // Display framerate info
        if (audio.is_playing()) {
            frame_count++;
            current_time = audio.get_current_time();
            if ((current_time-start_time)>=2) {
                
                printf("\rfps: %4.4f",frame_count/(current_time-start_time));
                fflush(stdout);
                frame_count = 0;
                start_time = current_time;
            }
        }

    }
    
    // Cleanup
    audio.pause();
    printf("\r\n");
    
    return 0;
}

