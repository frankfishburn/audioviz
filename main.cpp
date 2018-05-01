#include <cstdlib>
#include <exception>
#include <functional>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>

#include <matio.h>
#include <vector>

#include <iostream>
#include <memory>

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

SDL_Window* init_GL() {
    SDL_Window* wnd(
        SDL_CreateWindow("audioviz", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    auto glc = SDL_GL_CreateContext(wnd);

    auto rdr = SDL_CreateRenderer(
        wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        fprintf(stderr,"GL Error! %u\n",err);
    }
    
    return wnd;
}

void MyAudioCallback(void *userdata, Uint8 *stream, int len) {
    
    SDL_memset(stream, 0, len);
    
    Uint8** audio_ptr = (Uint8**) userdata;
    
    SDL_MixAudioFormat( stream, (Uint8*) *audio_ptr, AUDIO_F32, len, SDL_MIX_MAXVOLUME / 2);
    
    *audio_ptr += len;
    
}

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
    
    /* Set up audio playback */
    SDL_AudioSpec want, have;
    
    float *current_audio_ptr = input_data.signal;
    
    SDL_memset(&want, 0, sizeof(want));
    want.freq = sample_rate;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = 8192;
    want.callback = MyAudioCallback;
    want.userdata = (void*) &current_audio_ptr;
            
    if (SDL_OpenAudio(&want, &have) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    } else {
        if (have.format != want.format) {
            SDL_Log("We didn't get Float32 audio format.");
        }
    }
    
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
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        fprintf(stderr,"GL Error! %u\n",err);
        return 1;
    }
    
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
        
        while ((err = glGetError()) != GL_NO_ERROR) {
            cerr << "OpenGL error: " << err << endl;
        }
        
        SDL_GL_SwapWindow(wnd);
    };

    while(true) main_loop();
    
    return 0;
}

