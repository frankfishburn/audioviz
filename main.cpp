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
const char *vertex_tex_source = 
#include "shaders/vert_tex.glsl"
;
const char *fragment_tex_source = 
#include "shaders/frag_tex.glsl"
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
    GLint rgbUniform = glGetUniformLocation(shaderProgram, "RGB");
    
    // Set static uniforms
    GLint windurUniform = glGetUniformLocation(shaderProgram, "window_duration");
    GLint fsUniform = glGetUniformLocation(shaderProgram, "sample_rate");
    float window_duration = .05;
    glUniform1f( windurUniform, window_duration );
    glUniform1f(fsUniform, sample_rate);
    
    // Setup framebugger shaders
    GLuint screenShaderProgram = setup_shaders_source(vertex_tex_source, fragment_tex_source);
    
    // Set up vertex buffer object for interleaved audio data
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, num_channels*num_samples*sizeof(GLfloat), audio.get_data_ptr(), GL_STATIC_DRAW);
    
    // Set up vertex array object for each channel
    GLuint VAO[num_channels];
    glGenVertexArrays(num_channels, VAO );
    
    for (int channel=0; channel<num_channels; channel++) {
        
        glBindVertexArray( VAO[channel] );
        glEnableVertexAttribArray(ampAttrib);
        glVertexAttribPointer(ampAttrib, 1, GL_FLOAT, GL_FALSE, num_channels*sizeof(float), (void*) (channel * sizeof(float)) );
        
    }
    
    // Create a framebuffer
    GLuint offscreenBuffer, screenTexture, screenVAO, screenVBO;
    setup_framebuffer( wnd, &offscreenBuffer , &screenTexture , &screenVAO , &screenVBO );
    
    // Enable v-sync
    SDL_GL_SetSwapInterval(1);
    
    // Disable depth, enable alpha blending
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(false);
    
    // Check for GL errors
    glPrintErrors();
    
    // Setup timer
    double current_time;
    glUniform1f(timeUniform,0);
    
    // Start audio
    audio.play();
    
    unsigned long count = 0;
    
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
                        
                        destroy_framebuffer( &offscreenBuffer , &screenTexture , &screenVAO , &screenVBO );
                        setup_framebuffer( wnd, &offscreenBuffer , &screenTexture , &screenVAO , &screenVBO );
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                        glViewport(0, 0, e.window.data1, e.window.data2);
                        break;
                    }
                
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_SPACE: audio.toggle_playback(); break;
                        case SDLK_LEFT: audio.back(); break;
                        case SDLK_RIGHT: audio.forward(); break;
                    }
                    break;
                 
            }
        }

        // Bind framebuffer render target
        glBindFramebuffer(GL_FRAMEBUFFER, offscreenBuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        
        // Update current time
        current_time = audio.get_current_time();
        glUniform1f(timeUniform,current_time);
        
        // Update VBO indices to render
        int start_index = max( 0 , int ((current_time - window_duration/2) * sample_rate) );
        int end_index = min( int ((current_time + window_duration/2) * sample_rate) , num_samples );
        
        // Render each channel
        for (int channel=0; channel<num_channels; channel++){
            
            glBindVertexArray( VAO[channel] );
            
            if (channel==0){
                glUniform3f(rgbUniform, 1.0f, 0.0f, 0.0f);
            } else {
                glUniform3f(rgbUniform, 0.0f, 0.0f, 1.0f);
            }
            
            glDrawArrays(GL_LINE_STRIP, start_index, end_index-start_index );
        }
        
        // Render offscreen buffer to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(screenShaderProgram);
        glBindVertexArray(screenVAO);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        SDL_GL_SwapWindow(wnd);
        
        // Display framerate info
        if (audio.is_playing()) {
            count++;
            if (count%30==0) {
                printf("%lu frames, %f seconds, %f fps\n",count,current_time,count/current_time);
            }
        }
        
    };

    while(true) main_loop();
    
    return 0;
}

