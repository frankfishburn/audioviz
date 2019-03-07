#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "shader_program.h"
#include "window.h"

class FrameBuffer {
   public:
    FrameBuffer(Window* wnd, bool doMSAA);
    virtual ~FrameBuffer();
    void bind();
    void unbind();
    void draw();
    void freshen();
    int  width() { return width_; }
    int  height() { return height_; }
    void set_bloom(bool in) { do_bloom = in; }
    void toggle_bloom() { do_bloom = !do_bloom; }
    void toggle_msaa() {
        do_msaa = !do_msaa;
        freshen();
    }
    bool bloom_enabled() { return do_bloom; }
    bool msaa_enabled() { return do_msaa; }

   private:
    const int num_buffers = 2;
    int       num_samples;
    bool      do_bloom = true;
    bool      do_msaa  = true;
    GLenum    GL_TEXTURE_TYPE;
    Window*   window;
    GLuint    buffer[3];
    GLuint    texture[3];

    ShaderProgram* copy_shader;
    ShaderProgram* hblur_shader;
    ShaderProgram* vblur_shader;
    void           init();
    void           deinit();
    void           apply_bloom();
    int            width_;
    int            height_;
};

#endif /* FRAMEBUFFER_H */
