#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>

#include <fstream>
#include <string>

enum shader_types { vertex = GL_VERTEX_SHADER, fragment = GL_FRAGMENT_SHADER };

class Shader {
   public:
    Shader(shader_types shader_type);
    ~Shader();

    void compile(const std::string& shader_source);
    void compile(const std::ifstream& shader_file);
    void attach(GLuint);
    bool ready() const;

   private:
    GLuint object_;
    std::string get_messages();
};

#endif /* SHADER_H */
