#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <SDL2/SDL.h>

class Window {
   public:
    Window();
    ~Window();
    void check_errors();
    void swap();
    void toggle_fullscreen();
    int width() const;
    int height() const;

   private:
    int status;
    SDL_Window *window;
    SDL_GLContext context;
};

#endif /* WINDOW_H */