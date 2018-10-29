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
    
private:
    GLuint buffer;
    GLuint texture;
    GLuint VAO;
    GLuint VBO;
    ShaderProgram *shader;
    
};

#endif /* FRAMEBUFFER_H */

