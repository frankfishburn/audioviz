#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "shader_program.h"

class FrameBuffer {
   public:
    FrameBuffer(const int width, const int height, const bool do_msaa);
    virtual ~FrameBuffer();
    void bind() const;
    void unbind() const;
    void draw();
    void set_resolution(const int width, const int height);
    int width() const { return width_; }
    int height() const { return height_; }
    void set_bloom(bool in) { do_bloom = in; }
    void toggle_bloom() { do_bloom = !do_bloom; }
    void toggle_msaa() {
        do_msaa = !do_msaa;
        deinit();
        init();
    }
    void toggle_bg() { bg_black = !bg_black; };
    bool bloom_enabled() const { return do_bloom; }
    bool msaa_enabled() const { return do_msaa; }

   private:
    int width_;
    int height_;
    bool do_msaa;

    const int num_buffers = 2;
    int num_samples;
    bool do_bloom = true;
    bool bg_black = true;
    GLenum GL_TEXTURE_TYPE;
    GLuint buffer[3];
    GLuint texture[3];

    ShaderProgram copy_shader;
    ShaderProgram hblur_shader;
    ShaderProgram vblur_shader;
    void init();
    void deinit();
    void apply_bloom();
};

#endif /* FRAMEBUFFER_H */
