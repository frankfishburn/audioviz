#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <cstddef>
#include <GLES3/gl32.h>

class ShaderProgram {
public:
    ShaderProgram(const char* vertex_source, const char* fragment_source);
    ShaderProgram(const ShaderProgram& orig);
    virtual ~ShaderProgram();
    
    bool isok();
    void use();
    void print_errors();
    
    void set_uniform(const char*, int);
    void set_uniform(const char*, float);
    void set_uniform(const char*, float, float);
    void set_uniform(const char*, float, float, float);
    void set_attrib(const char*, size_t);
    
private:
    
    void compile_shader(GLuint &shader, const char *shaderSource);
    void link_shaders();
    
    GLuint program = 0;
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    
};

#endif /* SHADER_PROGRAM_H */

