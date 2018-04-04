#ifndef SHADERS_H
#define SHADERS_H

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>


GLuint setup_shaders(const char* vertex_shader_filename, const char* fragment_shader_filename);

#endif /* SHADERS_H */
