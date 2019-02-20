#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include <GL/glew.h>

class Window {

public:
    Window();
    ~Window();
    void check_errors();
    void swap();
    void toggle_fullscreen();
    int width();
    int height();
    
private:
    int status;
    SDL_Window* window;
    SDL_GLContext context;
};


#endif /* WINDOW_H */