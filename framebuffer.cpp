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
    
    // Setup FB triangles
    const float screenVertices[12] = {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };
    
    // screen quad VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
    shader->set_attrib("position", 2, 2*sizeof(float), 0);
    
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
    shader->use();
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
}