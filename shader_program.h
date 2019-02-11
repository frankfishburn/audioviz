#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <cstddef>
#include <string>
#include <GL/glew.h>

class ShaderProgram {
public:
    ShaderProgram(const char* vertex_source, const char* fragment_source);
    ShaderProgram(const ShaderProgram& orig);
    virtual ~ShaderProgram();
    
    bool isok();
    void use();
    void print_errors();
    GLuint get_program(){return program;}
    
    void set_uniform(const char*, int);
    void set_uniform(const char*, float);
    void set_uniform(const char*, float, float);
    void set_uniform(const char*, float, float, float);
    void set_attrib(const char*, int N, size_t stride, size_t offset);
    void set_attrib(const char*, int N, size_t stride);
    void set_attrib(const char*, int N);
    void set_attrib(const char*);
    
private:
    
    void compile_shader(GLuint &shader, const char *shaderSource);
    void link_shaders();
    
    GLuint program = 0;
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    std::string GLSL_version = "#version 410\n";
};

#endif /* SHADER_PROGRAM_H */

