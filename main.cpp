#include <cstdlib>
#include <exception>
#include <functional>

#include <vector>
#include <iostream>
#include <memory>

#include "audio_manager.h"

#include "opengl.h"
#include "shaders.h"

const char *vertex_source = 
#include "shaders/vert_direct.glsl"
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
    
    // Initialize window and context
    SDL_Window* wnd = init_GL();
    
    // Setup shaders
    GLuint shaderProgram = setup_shaders_source(vertex_source, fragment_source);
    GLint ampAttrib = glGetAttribLocation(shaderProgram, "amplitude");
    
    GLint timeUniform = glGetUniformLocation(shaderProgram, "current_time");
    GLint fsUniform = glGetUniformLocation(shaderProgram, "sample_rate");
    glUniform1f(fsUniform, sample_rate);
    
    GLint rgbUniform = glGetUniformLocation(shaderProgram, "RGB");
    
    // Add x-scaling factor based on time window
    float window_duration = .025;
    GLint windurUniform = glGetUniformLocation(shaderProgram, "window_duration");
    glUniform1f( windurUniform, window_duration );
    
    // Set up vertex buffer object for interleaved audio data
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, num_channels*num_samples*sizeof(GLfloat), audio.get_data_ptr(), GL_STATIC_DRAW);
    
    //glViewport(0,0,640,480);
    
    // Set up vertex array object for each channel
    GLuint VAO[num_channels];
    glGenVertexArraysOES(num_channels, VAO );
    
    for (int channel=0; channel<num_channels; channel++) {
        
        glBindVertexArrayOES( VAO[channel] );
        //glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(ampAttrib);
        glVertexAttribPointer(ampAttrib, 1, GL_FLOAT, GL_FALSE, num_channels*sizeof(float), (void*) (channel * sizeof(float)) );
        
    }
    
    // Check for GL errors
    glPrintErrors();
    
    // Setup timer
    Uint64 time_start = SDL_GetPerformanceCounter();
    float current_time;
    glUniform1f(timeUniform,0);
    
    // Start audio
    audio.play();
    
    // Render Loop
    loop = [&]
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT) std::abort();
        }
        
        // Clear the screen to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update current time
        current_time = (SDL_GetPerformanceCounter() - time_start) / (float) SDL_GetPerformanceFrequency();
        glUniform1f(timeUniform,current_time);
        
        // Render each channel
        for (int channel=0; channel<num_channels; channel++){
            
            glBindVertexArrayOES( VAO[channel] );
            
            if (channel==0){
                glUniform3f(rgbUniform, 1.0f, 0.0f, 0.0f);
            } else {
                glUniform3f(rgbUniform, 0.0f, 0.0f, 1.0f);
            }
            
            glDrawArrays(GL_LINE_STRIP, 0, num_samples);
        }
        
        SDL_GL_SwapWindow(wnd);
    };

    while(true) main_loop();
    
    return 0;
}

