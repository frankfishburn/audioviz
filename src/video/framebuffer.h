#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "shader_program.h"
#include "window.h"

class FrameBuffer {
   public:
    FrameBuffer(const Window &window, bool do_msaa);
    virtual ~FrameBuffer();
    void bind() const;
    void unbind() const;
    void draw();
    void freshen();
    int width() const { return width_; }
    int height() const { return height_; }
    void set_bloom(bool in) { do_bloom = in; }
    void toggle_bloom() { do_bloom = !do_bloom; }
    void toggle_msaa() {
        do_msaa = !do_msaa;
        freshen();
    }
    void toggle_bg() { bg_black = !bg_black; };
    bool bloom_enabled() { return do_bloom; }
    bool msaa_enabled() { return do_msaa; }

   private:
    const Window &window;
    bool do_msaa = true;

    const int num_buffers = 2;
    int num_samples;
    bool do_bloom = true;
    bool bg_black = true;
    GLenum GL_TEXTURE_TYPE;
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
