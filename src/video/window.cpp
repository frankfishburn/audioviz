#include "window.h"

GLboolean glewExperimental = GL_TRUE;

Window::Window() {
    status = 0;

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        status = -1;
    }

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow(
        "audioviz", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    context = SDL_GL_CreateContext(window);

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
    }

    SDL_GL_SetSwapInterval(0);
    glViewport(0, 0, 640, 480);

    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n\n", glGetString(GL_VERSION));
}

Window::~Window() {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
    SDL_Quit();
}

void Window::swap() { SDL_GL_SwapWindow(window); }

void Window::toggle_fullscreen() {
    bool isFullscreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
    if (isFullscreen) {
        SDL_SetWindowFullscreen(window, 0);
        SDL_ShowCursor(SDL_ENABLE);
    } else {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_ShowCursor(SDL_DISABLE);
    }
}

int Window::width() const {
    int width;
    SDL_GetWindowSize(window, &width, NULL);
    return width;
}

int Window::height() const {
    int height;
    SDL_GetWindowSize(window, NULL, &height);
    return height;
}

void Window::check_errors() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr, "GL Error! %u\n", err);
        status = err;
    }
    return;
}