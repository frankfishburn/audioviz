#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>

class Window {

public:
    Window();
    ~Window();
    void check_errors();
    void swap();
    void get_window_size(int* width, int* height);
    
private:
    int status;
    SDL_Window* window;
    SDL_GLContext context;
    SDL_Renderer* renderer;
};


#endif /* WINDOW_H */