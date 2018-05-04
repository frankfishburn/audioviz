#include <cstdlib>
#include <exception>
#include <functional>

#include <vector>
#include <iostream>
#include <memory>

#include "opengl.h"
#include "audio_playback.h"
#include "shaders.h"
#include "load_audio.h"

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
    
    /* Load audio data */
    audio_data input_data;
    load_audio( argv[1] , &input_data );
    
    const unsigned int sample_rate = input_data.sample_rate;
    const unsigned int num_samples = input_data.num_samples;
    const unsigned int num_channels = input_data.num_channels;
        
    /*  Initialize window and context  */
    SDL_Window* wnd = init_GL();
    
    /*  Setup shaders  */
    GLuint shaderProgram = setup_shaders_source(vertex_source, fragment_source);
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    
    GLint timeUniform = glGetUniformLocation(shaderProgram, "current_time");
    GLint fsUniform = glGetUniformLocation(shaderProgram, "sample_rate");
    glUniform1f(fsUniform, sample_rate);
    
    /* Add x-scaling factor based on time window */
    float window_duration = .025;
    GLint windurUniform = glGetUniformLocation(shaderProgram, "window_duration");
    glUniform1f( windurUniform, window_duration );
    
    /* Set up vertex array object */
    GLuint VAO;
    glGenVertexArraysOES(1, &VAO );
    glBindVertexArrayOES( VAO );
    
    /* Set up vertex buffer objects */
    GLuint vbo[num_channels];
    glGenBuffers(num_channels, vbo);
    for (int channel=0; channel<num_channels; channel++) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[channel]);
        glBufferData(GL_ARRAY_BUFFER, num_samples*sizeof(GLfloat), input_data.signal+channel*num_samples, GL_STATIC_DRAW);
    }
    glEnableVertexAttribArray(posAttrib);
    
    /* Check for GL errors */
    glPrintErrors();
    
    /* Setup timer */
    Uint64 time_start = SDL_GetPerformanceCounter();
    float current_time;
    glUniform1f(timeUniform,0);
    
    /* Start audio */
    SDL_PauseAudio(0);
    
    /*  Render Loop  */
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
        
            glBindBuffer(GL_ARRAY_BUFFER, vbo[channel]);
            glVertexAttribPointer(posAttrib, 1, GL_FLOAT, GL_FALSE, 0, 0);
            glDrawArrays(GL_LINE_STRIP, 0, num_samples);
        }
                
        SDL_GL_SwapWindow(wnd);
    };

    while(true) main_loop();
    
    return 0;
}

