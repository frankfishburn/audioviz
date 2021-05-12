#include "shader.h"

#include <sstream>
#include <vector>

static constexpr char GLSL_version[] = "#version 410 core\n";

Shader::Shader(shader_types shader_type) {
    object_ = glCreateShader(shader_type);
}

Shader::~Shader() { glDeleteShader(object_); }

void Shader::compile(const std::string& shader_source) {
    // Create full source
    std::string source;
    source.append(GLSL_version);
    source.append(shader_source);
    const char* source_ptr = source.c_str();

    // Try to compile
    glShaderSource(object_, 1, &source_ptr, NULL);
    glCompileShader(object_);

    // Check if it was successful
    if (!ready()) throw "Shader compilation failed.\n" + get_messages();
}

void Shader::compile(const std::ifstream& shader_file) {
    if (!shader_file.is_open()) throw "Could not open shader file";

    std::ostringstream ss;
    ss << shader_file.rdbuf();
    compile(ss.str());
}

void Shader::attach(GLuint program) { glAttachShader(program, object_); }

bool Shader::ready() const {
    GLint compiled;
    glGetShaderiv(object_, GL_COMPILE_STATUS, &compiled);
    return compiled == GL_TRUE;
}

std::string Shader::get_messages() {
    GLint maxLength = 0;
    glGetShaderiv(object_, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> infoLog(maxLength);
    glGetShaderInfoLog(object_, maxLength, &maxLength, &infoLog[0]);

    std::string status;
    for (auto i : infoLog) status.push_back(i);
    return status;
}
