#include "shader_program.h"

#include <fstream>
#include <iostream>
#include <vector>

ShaderProgram::ShaderProgram() { program = glCreateProgram(); }

ShaderProgram::~ShaderProgram() { glDeleteProgram(program); }

void ShaderProgram::compile(const std::string &vertex_source,
                            const std::string &fragment_source) {
    vertex_shader.compile(vertex_source);
    fragment_shader.compile(fragment_source);

    vertex_shader.attach(program);
    fragment_shader.attach(program);

    glLinkProgram(program);

    if (!ready()) throw "Could not link shader program.\n" + get_messages();
}

void ShaderProgram::compile(const std::ifstream &vertex_file,
                            const std::ifstream &fragment_file) {
    vertex_shader.compile(vertex_file);
    fragment_shader.compile(fragment_file);

    vertex_shader.attach(program);
    fragment_shader.attach(program);

    glLinkProgram(program);

    if (!ready()) throw "Could not link shader program.\n" + get_messages();
}

bool ShaderProgram::ready() const {
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    return linked == GL_TRUE;
}

void ShaderProgram::use() const {
    if (!ready()) throw "Cannot use a shader program that isn't ready";
    glUseProgram(program);
}

void ShaderProgram::set_uniform(const char *name, int value) {
    glUseProgram(program);
    GLint uniform = glGetUniformLocation(program, name);
    glUniform1i(uniform, value);
}

void ShaderProgram::set_uniform(const char *name, float value) {
    glUseProgram(program);
    GLint uniform = glGetUniformLocation(program, name);
    glUniform1f(uniform, value);
}

void ShaderProgram::set_uniform(const char *name, float value1, float value2) {
    glUseProgram(program);
    GLint uniform = glGetUniformLocation(program, name);
    glUniform2f(uniform, value1, value2);
}

void ShaderProgram::set_uniform(const char *name, float value1, float value2,
                                float value3) {
    glUseProgram(program);
    GLint uniform = glGetUniformLocation(program, name);
    glUniform3f(uniform, value1, value2, value3);
}

void ShaderProgram::set_attrib(const char *name) { set_attrib(name, 1, 0, 0); }

void ShaderProgram::set_attrib(const char *name, int N) {
    set_attrib(name, N, N * sizeof(float), 0);
}

void ShaderProgram::set_attrib(const char *name, int N, size_t stride) {
    set_attrib(name, N, stride, 0);
}

void ShaderProgram::set_attrib(const char *name, int N, size_t stride,
                               size_t offset) {
    glUseProgram(program);
    GLint attrib = glGetAttribLocation(program, name);
    glEnableVertexAttribArray(attrib);
    glVertexAttribPointer(attrib, N, GL_FLOAT, GL_FALSE, stride,
                          (void *)offset);
}

std::string ShaderProgram::get_messages() {
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> infoLog(maxLength);
    glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

    std::string status;
    for (auto i : infoLog) status.push_back(i);
    return status;
}
