#include "vertex_array.h"

#include <GL/glew.h>

static GLuint create() {
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    return VAO;
}

VertexArray::VertexArray() : VAO_(create()) {}

VertexArray::~VertexArray() { glDeleteVertexArrays(1, &VAO_); }

void VertexArray::bind() const { glBindVertexArray(VAO_); }
