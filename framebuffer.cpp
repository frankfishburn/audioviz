#include "framebuffer.h"

const char *src_copy_vert = 
#include "shaders/copy_vert.glsl"
;
const char *src_copy_frag = 
#include "shaders/copy_frag.glsl"
;
const char *src_blurh_frag = 
#include "shaders/blurh_frag.glsl"
;
const char *src_blurv_frag = 
#include "shaders/blurv_frag.glsl"
;

FrameBuffer::FrameBuffer(Window* wnd) {

    window = wnd;
    
    // Setup copy shader
    copy_shader = new ShaderProgram(src_copy_vert, src_copy_frag);
    copy_shader->set_uniform("num_samples",num_samples);
    
    // Setup horizontal and vertical blur shaders
    hblur_shader = new ShaderProgram(src_copy_vert, src_blurh_frag);
    hblur_shader->set_uniform("blur_radius",1.0f);
    
    vblur_shader = new ShaderProgram(src_copy_vert, src_blurv_frag);
    vblur_shader->set_uniform("blur_radius",1.0f);
    
    // Initialize framebuffer
    init();
    
}


FrameBuffer::~FrameBuffer() {

    deinit();
    delete copy_shader;
    delete hblur_shader;
    delete vblur_shader;
    
}

void FrameBuffer::init(){
        
    // Get window size
    width_ = window->width();
    height_ = window->height();
    GLenum status;
    
    // Create 2 textures and framebuffers
    for (int i=0; i<2; i++) {
        
        // Create a FrameBuffer
        glGenFramebuffers(1, &buffer[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer[i]);

        // Create the texture
        glGenTextures(1, &texture[i]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture[i]);
        glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_RGBA8,  width_, height_, GL_FALSE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture[i], 0);

        // Check FrameBuffer
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
        if(status != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr,"GL FrameBuffer Error! %u\n",status);
            return;
        }
    }
    
    // Reset buffers
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void FrameBuffer::deinit() {
    
    glDeleteTextures(1, &texture[0]);
    glDeleteTextures(1, &texture[1]);
    glDeleteFramebuffers(1, &buffer[0]);
    glDeleteFramebuffers(1, &buffer[1]);
    
}

void FrameBuffer::freshen() {
    deinit();
    init();
}

void FrameBuffer::bind() {
    
    // Bind FrameBuffer render target
    glBindFramebuffer(GL_FRAMEBUFFER, buffer[0]);
    glViewport(0,0,width_,height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
}

void FrameBuffer::unbind() {
    
    // Unbind FrameBuffer render target
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0,window->width(),window->height());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

}

void FrameBuffer::draw() {
    
    apply_bloom();
    
    // Render offscreen buffer to screen
    glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0,0,width_,height_,0,0,window->width(),window->height(),GL_COLOR_BUFFER_BIT,GL_LINEAR);
    
}

void FrameBuffer::apply_bloom() {
    
    // Bind second buffer as render target
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer[1]);
    glViewport(0,0,width_,height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Apply horizontal blur to original data
    copy_shader->use();
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture[0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Bind first buffer as render target
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer[0]);
    glViewport(0,0,width_,height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Apply vertical blur to horizontally-blurred data
    copy_shader->use();
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture[1]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    unbind();
    
}
