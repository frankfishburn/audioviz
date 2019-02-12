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
    void toggle_bloom() { do_bloom = !do_bloom; }
    
private:
    const int num_samples = 1;
    const int num_buffers = 3;
    bool do_bloom = true;
    GLenum GL_TEXTURE_TYPE;
    Window* window;
    GLuint buffer[3];
    GLuint texture[3];
    
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

