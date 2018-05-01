#ifndef SHADERS_H
#define SHADERS_H

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>

GLuint setup_shaders_source(const char* vs_source, const char* fs_source);
GLuint setup_shaders_filename(const char* vs_filename, const char* fs_filename);

#endif /* SHADERS_H */
