#ifndef OPENGL_H
#define OPENGL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>

void glPrintErrors();

SDL_Window* init_GL();

#endif /* OPENGL_H */