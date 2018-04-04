#include "shaders.h"

#include <cstdlib>
#include <fstream>
#include <vector>
#include <iostream>


std::string read_file(const char* filename){
    
    std::string line, text;
    
    /* Open vertex shader file */
    std::ifstream shader_file(filename);
    if (!shader_file.is_open()) {
        fprintf(stderr,"Couldn''t open shader file: %s",filename);
        return text;
    }
    
    /* Load vertex shader source */
    while(std::getline(shader_file, line)) {
        text += line + "\n";
    }
    
    shader_file.close();
    
    return text;
    
}

GLuint compile_shader(const char *shaderSource, GLenum shaderType){
    
    // Create shader and compile
    GLuint shader = glCreateShader(shaderType);
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
    }
    
    return shader;
}

GLuint link_shaders(GLuint *vertexShader, GLuint *fragmentShader) {
    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, *vertexShader);
    glAttachShader(shaderProgram, *fragmentShader);
    // glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    
    GLint linked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    
    if (!linked)
    {
        GLint maxLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
        
        fprintf(stderr,"Shader linking failed\n");
        for (auto i: infoLog)            
            std::cerr << i;
        
        fprintf(stderr,"\n\n");
        
	glDeleteProgram(shaderProgram);
        
    }
    
    return shaderProgram;
    
}

GLuint setup_shaders(const char* vs_filename, const char* fs_filename) {

    // Load shader files
    std::string vertexSource = read_file(vs_filename);
    std::string fragmentSource = read_file(fs_filename);
    
    // Compile shaders
    GLuint vertexShader = compile_shader(vertexSource.c_str(),GL_VERTEX_SHADER);
    GLuint fragmentShader = compile_shader(fragmentSource.c_str(),GL_FRAGMENT_SHADER);
    
    // Link shader program
    GLuint shaderProgram = link_shaders(&vertexShader, &fragmentShader);
    
    // Use shader program
    glUseProgram(shaderProgram);
    
    // Check for errors
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
        fprintf(stderr,"Program GL Error! %u\n",err);
    
    return shaderProgram;
}
