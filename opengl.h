#ifndef OPENGL_H
#define OPENGL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <GLES3/gl3.h>
#include <SDL2/SDL_opengles2.h>

void glPrintErrors();

SDL_Window* init_GL();
void setup_framebuffer( GLuint *framebuffer , GLuint *texture , GLuint *VAO , GLuint *shaderProgram );

#endif /* OPENGL_H */