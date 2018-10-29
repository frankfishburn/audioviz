#include "shader_program.h"

#include <cstdlib>
#include <fstream>
#include <vector>
#include <iostream>

ShaderProgram::ShaderProgram(const char* vertex_source, const char* fragment_source) {
    
    // Allocate shaders
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Compile shaders
    compile_shader(vertex_shader, vertex_source);
    compile_shader(fragment_shader, fragment_source);
    
    // Link shader program
    link_shaders();
    
}

/*
ShaderProgram::ShaderProgram(const ShaderProgram& orig) {
}
*/

ShaderProgram::~ShaderProgram() {
    
    glDeleteProgram(program);
    
}

bool ShaderProgram::isok() {
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    return (linked==1);
}

void ShaderProgram::use() {
    glUseProgram(program);
}

void ShaderProgram::print_errors() {
    
    // Print GL errors
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr,"Program GL Error! %u\n",err);
    }

}

void ShaderProgram::compile_shader(GLuint &shader, const char *shaderSource){
    
    // Create shader and compile
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    // Check shader
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    
    if(!compiled) {
        
	GLint maxLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
	std::vector<GLchar> infoLog(maxLength);
	glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
        
        fprintf(stderr,"Shader compilation failed\n");
        fprintf(stderr,"Shader source:\n%s\n\n",shaderSource);
        for (auto i: infoLog)            
            std::cerr << i;
        fprintf(stderr,"\n\n");
        
	glDeleteShader(shader);
    } else {
        printf("Compiled OK\n");
    }
    
}

void ShaderProgram::link_shaders() {
    
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    
    glLinkProgram(program);
    
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    
    if (!linked) {
    
        // Print shader linking errors
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        fprintf(stderr,"Shader linking failed\n");
        for (auto i: infoLog)            
            std::cerr << i;

        fprintf(stderr,"\n\n");
    
    } else {
        
        printf("Linked OK\n");
        use();
    
    }
}

void ShaderProgram::set_uniform(const char* name, int value) {
    
    GLint uniform = glGetUniformLocation(program, name);
    glUniform1i(uniform, value);
    
}

void ShaderProgram::set_uniform(const char* name, float value) {
    
    GLint uniform = glGetUniformLocation(program, name);
    glUniform1f(uniform, value);

}

void ShaderProgram::set_uniform(const char* name, float value1, float value2, float value3) {

    GLint uniform = glGetUniformLocation(program, name);
    glUniform3f(uniform, value1, value2, value3);

}

void ShaderProgram::set_attrib(const char* name, size_t size) {
    
    GLint attrib = glGetAttribLocation(program, name);
    glEnableVertexAttribArray(attrib);
    glVertexAttribPointer(attrib, 1, GL_FLOAT, GL_FALSE, size, 0 );
    
}

/*
int ShaderProgram::setup_shaders_source(GLuint &shaderProgram, const char* vs_source, const char* fs_source) {
    
    // Compile shaders
    GLuint vertexShader = compile_shader(vs_source,GL_VERTEX_SHADER);
    GLuint fragmentShader = compile_shader(fs_source,GL_FRAGMENT_SHADER);
    
    // Link shader program
    shaderProgram = link_shaders(&vertexShader, &fragmentShader);
    
    // Use shader program
    glUseProgram(shaderProgram);
    
    // Check for errors
    int status = 0;
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr,"Program GL Error! %u\n",err);
        status = 1;
    }
    
    return status;
}

int ShaderProgram::setup_shaders_filename(GLuint &shaderProgram, const char* vs_filename, const char* fs_filename) {

    // Load shader files
    std::string vertexSource = read_file(vs_filename);
    std::string fragmentSource = read_file(fs_filename);
    
    return setup_shaders_source(shaderProgram, vertexSource.c_str(), fragmentSource.c_str());
}

std::string ShaderProgram::read_txt_file(const char* filename){
    
    std::string line, text;
    
    // Open vertex shader file
    std::ifstream shader_file(filename);
    if (!shader_file.is_open()) {
        fprintf(stderr,"Couldn''t open shader file: %s",filename);
        return text;
    }
    
    // Load vertex shader source
    while(std::getline(shader_file, line)) {
        text += line + "\n";
    }
    
    shader_file.close();
    
    return text;
    
}
*/