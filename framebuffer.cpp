#include "framebuffer.h"

const char *vertex_tex_source = 
#include "shaders/vert_tex.glsl"
;
const char *fragment_tex_source = 
#include "shaders/frag_tex.glsl"
;

FrameBuffer::FrameBuffer(Window* wnd) {

    // Compile/link shader
    shader = new ShaderProgram(vertex_tex_source, fragment_tex_source);
    
    // Initialize framebuffer
    window = wnd;
    init();
    
}


FrameBuffer::~FrameBuffer() {

    deinit();
    delete shader;
    
}

void FrameBuffer::init(){
    
    // Create a FrameBuffer
    glGenFramebuffers(1, &buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer);
    
    // Get window size
    width_ = window->width();
    height_ = window->height();
    
    // Create the texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    // Check FrameBuffer
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr,"GL FrameBuffer Error! %u\n",status);
        return;
    }
    
    // Reset buffers
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void FrameBuffer::deinit() {
    
    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &buffer);
    
}

void FrameBuffer::freshen() {
    deinit();
    init();
}

void FrameBuffer::bind() {
    
    // Bind FrameBuffer render target
    glBindFramebuffer(GL_FRAMEBUFFER, buffer);
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
    
    // Render offscreen buffer to screen
    shader->use();
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    
}