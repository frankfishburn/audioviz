#include <cstdlib>
#include <exception>
#include <functional>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>

#include <matio.h>
#include <vector>

#include <iostream>
#include <memory>

#include "shaders.h"
#include "load_audio.h"

using namespace std;

std::function<void()> loop;
void main_loop() { loop(); }

SDL_Window* init_GL() {
    SDL_Window* wnd(
        SDL_CreateWindow("audioviz", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    auto glc = SDL_GL_CreateContext(wnd);

    auto rdr = SDL_CreateRenderer(
        wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        fprintf(stderr,"GL Error! %i\n",err);
    }
    
    return wnd;
}

struct Mesh {
    GLuint VAO;
    unsigned long numVertices;
    GLenum renderMode;
};

class VertexManager
{
    private:
        std::vector<std::unique_ptr<Mesh>> Meshes;
        GLint *posAttrib;
        
    public:
        void setPosAttrib(GLint*);
        void addMesh( GLfloat* vertices, unsigned long numVertices, GLenum renderMode );
        void render();
};

void VertexManager::setPosAttrib(GLint* NewPosAttrib)
{
    posAttrib = NewPosAttrib;
}

void VertexManager::addMesh( GLfloat *vertices, unsigned long numVertices, GLenum renderMode )
{
    
    Meshes.push_back(std::unique_ptr<Mesh>(new Mesh));
    Meshes.back().get()->numVertices = numVertices;
    Meshes.back().get()->renderMode = renderMode;
        
    glGenVertexArraysOES(1, &Meshes.back().get()->VAO );
    glBindVertexArrayOES( Meshes.back().get()->VAO );
    
    std::shared_ptr<GLuint> vbo = std::make_shared<GLuint>();
    glGenBuffers(1, vbo.get());
    
    
    
    glBindBuffer(GL_ARRAY_BUFFER, *vbo.get());
    glBufferData(GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    
    // Specify the layout of the vertex data
    glEnableVertexAttribArray(*posAttrib);
    glVertexAttribPointer(*posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
}

void VertexManager::render()
{
    for (unsigned long i=0; i<Meshes.size(); i++)
    {
        glBindVertexArrayOES(Meshes.at(i).get()->VAO);
        glDrawArrays(Meshes.at(i).get()->renderMode, 0, Meshes.at(i).get()->numVertices);
    }
}

int main(int argc, char** argv) {
    
    /* Load audio data */
    audio_data input_data;
    load_audio( "/tmp/tmp.ogg" , &input_data );
        
    /*  Initialize window and context  */
    SDL_Window* wnd = init_GL();
    
    /*  Setup shaders  */
    GLuint shaderProgram = setup_shaders("shaders/vert_direct.glsl", "shaders/frag_plain.glsl");
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    
    /* Check for GL errors */
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        fprintf(stderr,"GL Error! %i\n",err);
        return 1;
    }
    
    
    /* Setup the vertex manager */
    VertexManager vertman;
    vertman.setPosAttrib(&posAttrib);
    
    /* Add signal mesh */
    vertman.addMesh( input_data.signal , input_data.num_samples , GL_LINE_STRIP );
    
    
    // Reset buffer
    //glBindBuffer(GL_ARRAY_BUFFER,0);
    //glBindVertexArrayOES(0);
    
    
    /*  Render Loop  */
    loop = [&]
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT) std::abort();
        }
        
        // Clear the screen to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        vertman.render();

        while ((err = glGetError()) != GL_NO_ERROR) {
            cerr << "OpenGL error: " << err << endl;
        }
        
        SDL_GL_SwapWindow(wnd);
    };

    while(true) main_loop();
    
    return 0;
}

