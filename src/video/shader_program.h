#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <GL/glew.h>

#include <cstddef>
#include <fstream>
#include <string>

#include "shader.h"

class ShaderProgram {
   public:
    ShaderProgram();
    ~ShaderProgram();

    void compile(const std::string &vertex_source,
                 const std::string &fragment_source);
    void compile(const std::ifstream &vertex_file,
                 const std::ifstream &fragment_file);

    bool ready() const;
    void use() const;

    void set_uniform(const char *, int);
    void set_uniform(const char *, float);
    void set_uniform(const char *, float, float);
    void set_uniform(const char *, float, float, float);
    void set_attrib(const char *, int N, size_t stride, size_t offset);
    void set_attrib(const char *, int N, size_t stride);
    void set_attrib(const char *, int N);
    void set_attrib(const char *);

   private:
    GLuint program;
    Shader vertex_shader = Shader(vertex);
    Shader fragment_shader = Shader(fragment);
    std::string get_messages();
};

#endif /* SHADER_PROGRAM_H */
