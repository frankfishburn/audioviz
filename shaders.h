#ifndef SHADERS_H
#define SHADERS_H

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>

int setup_shaders_source(GLuint &shaderProgram, const char* vs_source, const char* fs_source);
int setup_shaders_filename(GLuint &shaderProgram, const char* vs_filename, const char* fs_filename);

#endif /* SHADERS_H */
