#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "shader_program.h"
#include "window.h"

class FrameBuffer {
public:
    FrameBuffer(Window* wnd);
    virtual ~FrameBuffer();
    void bind();
    void unbind();
    void draw();
    void freshen();
    
private:
    Window* window;
    GLuint buffer;
    GLuint texture;
    GLuint VAO;
    GLuint VBO;
    ShaderProgram *shader;
    void init();
    void deinit();
};

#endif /* FRAMEBUFFER_H */

