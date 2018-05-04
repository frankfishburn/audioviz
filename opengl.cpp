#include "opengl.h"

void glPrintErrors(){
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        fprintf(stderr,"GL Error! %u\n",err);
    }
    return;
}

SDL_Window* init_GL() {
    SDL_Window* wnd(
        SDL_CreateWindow("audioviz", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    auto glc = SDL_GL_CreateContext(wnd);

    auto rdr = SDL_CreateRenderer(
        wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    glPrintErrors();

    return wnd;
}

