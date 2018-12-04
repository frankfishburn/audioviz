#include "window.h"
#include <GLES3/gl3.h>

Window::Window() {
    
    status = 0;
    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        status = -1;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    
    SDL_GL_SetSwapInterval(0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    window = SDL_CreateWindow("audioviz", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    
    context = SDL_GL_CreateContext(window);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
        
}

Window::~Window() {
    
    SDL_DestroyRenderer(renderer);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
    SDL_Quit();
    
}

void Window::swap() {
    SDL_GL_SwapWindow(window);
}

void Window::get_window_size(int* width, int* height) {
    SDL_GetWindowSize(window, width, height);
}

void Window::check_errors(){
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        fprintf(stderr,"GL Error! %u\n",err);
        status = err;
    }
    return;
}