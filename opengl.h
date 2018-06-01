#ifndef OPENGL_H
#define OPENGL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <GLES3/gl3.h>
#include <SDL2/SDL_opengles2.h>

void glPrintErrors();

SDL_Window* init_GL();
int setup_framebuffer( SDL_Window *wnd, GLuint *framebuffer , GLuint *texture , GLuint *VAO , GLuint *VBO );
void destroy_framebuffer(GLuint *framebuffer , GLuint *texture , GLuint *VAO , GLuint *VBO );

#endif /* OPENGL_H */