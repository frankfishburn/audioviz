#include "vertex_buffer.h"

#include <GL/glew.h>

static GLuint create() {
    GLuint VBO;
    glGenBuffers(1, &VBO);
    return VBO;
}

VertexBuffer::VertexBuffer(const unsigned long size)
    : VBO_(create()), size_(size) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, size_ * sizeof(GLfloat), NULL,
                 GL_DYNAMIC_DRAW);
}

VertexBuffer::~VertexBuffer() { glDeleteBuffers(1, &VBO_); }

void VertexBuffer::bind() const { glBindBuffer(GL_ARRAY_BUFFER, VBO_); }

void VertexBuffer::push(const float* data) const {
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size_ * sizeof(GLfloat), data);
}
