#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#include "shader_program.h"

class FrameBuffer {
public:
    FrameBuffer(SDL_Window *wnd);
    virtual ~FrameBuffer();
    void bind();
    void unbind();
    void draw();
    void freshen();
    
private:
    SDL_Window* window;
    GLuint buffer;
    GLuint texture;
    GLuint VAO;
    GLuint VBO;
    ShaderProgram *shader;
    void init();
    void deinit();
};

#endif /* FRAMEBUFFER_H */

