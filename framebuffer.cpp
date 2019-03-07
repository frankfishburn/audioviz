#include "framebuffer.h"

const char* src_copy_vert =
#include "shaders/copy_vert.glsl"
    ;
const char* src_copy_frag_noMSAA =
#include "shaders/copy_frag_noMSAA.glsl"
    ;
const char* src_blurh_frag_noMSAA =
#include "shaders/blurh_frag_noMSAA.glsl"
    ;
const char* src_blurv_frag_noMSAA =
#include "shaders/blurv_frag_noMSAA.glsl"
    ;

const char* src_copy_frag_MSAA =
#include "shaders/copy_frag_MSAA.glsl"
    ;
const char* src_blurh_frag_MSAA =
#include "shaders/blurh_frag_MSAA.glsl"
    ;
const char* src_blurv_frag_MSAA =
#include "shaders/blurv_frag_MSAA.glsl"
    ;

FrameBuffer::FrameBuffer(Window* wnd, bool doMSAA) {
    window  = wnd;
    do_msaa = doMSAA;

    // Initialize framebuffer
    init();
}

FrameBuffer::~FrameBuffer() {
    deinit();
    delete copy_shader;
    delete hblur_shader;
    delete vblur_shader;
}

void FrameBuffer::init() {
    if (do_msaa) {  // MSAA

        num_samples     = 5;
        GL_TEXTURE_TYPE = GL_TEXTURE_2D_MULTISAMPLE;

        // Setup copy shader
        copy_shader = new ShaderProgram(src_copy_vert, src_copy_frag_MSAA);
        copy_shader->set_uniform("num_samples", num_samples);

        // Setup horizontal and vertical blur shaders
        hblur_shader = new ShaderProgram(src_copy_vert, src_blurh_frag_MSAA);
        hblur_shader->set_uniform("num_samples", num_samples);

        vblur_shader = new ShaderProgram(src_copy_vert, src_blurv_frag_MSAA);
        vblur_shader->set_uniform("num_samples", num_samples);

    } else {  // No MSAA

        num_samples     = 1;
        GL_TEXTURE_TYPE = GL_TEXTURE_2D;

        // Setup copy shader
        copy_shader = new ShaderProgram(src_copy_vert, src_copy_frag_noMSAA);

        // Setup horizontal and vertical blur shaders
        hblur_shader = new ShaderProgram(src_copy_vert, src_blurh_frag_noMSAA);
        vblur_shader = new ShaderProgram(src_copy_vert, src_blurv_frag_noMSAA);
    }

    // Get window size
    width_  = window->width();
    height_ = window->height();
    GLenum status;

    // Create 2 textures and framebuffers
    for (int i = 0; i < num_buffers; i++) {
        // Create a FrameBuffer
        glGenFramebuffers(1, &buffer[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer[i]);

        // Create the texture
        glGenTextures(1, &texture[i]);
        glBindTexture(GL_TEXTURE_TYPE, texture[i]);
        if (num_samples > 1) {
            glTexStorage2DMultisample(GL_TEXTURE_TYPE, num_samples, GL_RGBA8, width_, height_, GL_FALSE);
        } else {
            glTexStorage2D(GL_TEXTURE_TYPE, num_samples, GL_RGBA8, width_, height_);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_TYPE, texture[i], 0);

        // Check FrameBuffer
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "GL FrameBuffer %i Error! %u\n", i, status);
            return;
        }
    }

    // Reset buffers
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::deinit() {
    for (int i = 0; i < num_buffers; i++) {
        glDeleteTextures(1, &texture[i]);
        glDeleteFramebuffers(1, &buffer[i]);
    }
}

void FrameBuffer::freshen() {
    deinit();
    init();
}

void FrameBuffer::bind() {
    // Bind FrameBuffer render target
    glBindFramebuffer(GL_FRAMEBUFFER, buffer[0]);
    glViewport(0, 0, width_, height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void FrameBuffer::unbind() {
    // Unbind FrameBuffer render target
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window->width(), window->height());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void FrameBuffer::draw() {
    if (do_bloom) {
        apply_bloom();
    }

    // Render offscreen buffer to screen
    glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width_, height_, 0, 0, window->width(), window->height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void FrameBuffer::apply_bloom() {
    // 1) texture 0 -> horizontal blur -> buffer 1
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer[1]);
    glViewport(0, 0, width_, height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    hblur_shader->use();
    glBindTexture(GL_TEXTURE_TYPE, texture[0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // 2) texture 1 -> vertical blur --blend--> buffer 0
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer[0]);
    glViewport(0, 0, width_, height_);

    glEnable(GL_BLEND);  // Blend the blurred result with the original data
    glBlendColor(0, 0, 0, 0.5);
    glBlendFunc(GL_CONSTANT_ALPHA, GL_CONSTANT_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    vblur_shader->use();
    glBindTexture(GL_TEXTURE_TYPE, texture[1]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisable(GL_BLEND);
    unbind();
}
