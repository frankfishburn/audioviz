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
    int width() {return width_;}
    int height() {return height_;}
    
private:
    Window* window;
    GLuint buffer[2];
    GLuint texture[2];
    
    ShaderProgram *copy_shader;
    ShaderProgram *hblur_shader;
    ShaderProgram *vblur_shader;
    void init();
    void deinit();
    void apply_bloom();
    int width_;
    int height_;
};

#endif /* FRAMEBUFFER_H */

