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
        
    // Get window size
    width_ = window->width();
    height_ = window->height();
    
    // Create the texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 5, GL_RGBA8, width_, height_, GL_FALSE);
    
    // Create a FrameBuffer
    glGenFramebuffers(1, &buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture, 0);
    
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
    
    /*
    const float screenVertices[32] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    // screen quad VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
     */
    
}

void FrameBuffer::deinit() {
    
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
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
    glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0,0,width_,height_,0,0,window->width(),window->height(),GL_COLOR_BUFFER_BIT,GL_LINEAR);
    
    // Don't need any of this right now since no overlay/postprocessing effects yet
    //shader->use();
    //glBindVertexArray(VAO);
    //glDisable(GL_DEPTH_TEST);
    //glBindTexture(GL_TEXTURE_2D, texture);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    
}