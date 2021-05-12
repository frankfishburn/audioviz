#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "algorithm/stft.h"
#include "audio/file_source.h"
#include "audio/player.h"
#include "vfx/eclipse/effect.h"
#include "video/framebuffer.h"
#include "video/shader_program.h"
#include "video/window.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Please supply a filename to a music file.\n");
        return EXIT_FAILURE;
    }

    // Load audio file
    FileAudioSource audio_source;
    try {
        audio_source.open(argv[1]);
    } catch (const AudioSourceError& e) {
        std::cerr << "Error loading audio source:" << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "Exiting" << std::endl;
        return EXIT_FAILURE;
    }

    AudioPlayer audio_player = AudioPlayer(audio_source);

    if (!audio_player.playable() || audio_source.num_samples() == 0) {
        std::cerr << "Audio problem, bailing out!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Playing " << audio_source.description() << std::endl;

    // Initialize window and context
    Window window;

    // Create a framebuffer
    FrameBuffer fb(window, true);

    // Setup visual effect renderer
    FXEclipse vfx(audio_source, fb);

    // Enable v-sync
    SDL_GL_SetSwapInterval(1);

    // Disable depth, enable alpha blending
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(false);

    // Check for GL errors
    window.check_errors();

    // Start audio
    audio_player.play();
    double start_time = 0;
    double current_time = 0;
    std::string current_time_str;
    unsigned long frame_count = 0;
    bool force_refresh = true;

    // Render Loop
    bool quit = false;
    while (!quit) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        fb.freshen();
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                        glViewport(0, 0, e.window.data1, e.window.data2);
                        vfx.set_resolution(fb.width(), fb.height());
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_SPACE:
                            audio_player.toggle_playback();
                            break;
                        case SDLK_LEFT:
                            audio_player.back();
                            start_time = audio_player.current_time();
                            frame_count = 0;
                            force_refresh = true;
                            break;
                        case SDLK_RIGHT:
                            audio_player.forward();
                            start_time = audio_player.current_time();
                            frame_count = 0;
                            force_refresh = true;
                            break;
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                        case SDLK_q:
                            quit = true;
                            break;
                        case SDLK_f:
                            window.toggle_fullscreen();
                            break;
                        case SDLK_b:
                            fb.toggle_bloom();
                            break;
                        case SDLK_m:
                            fb.toggle_msaa();
                            break;
                        case SDLK_k:
                            fb.toggle_bg();
                            break;
                    }
                    break;
            }
        }

        // Render visual effects for current position into framebuffer
        long current_sample = audio_player.current_sample();
        vfx.draw(current_sample);

        // Draw framebuffer to screen
        fb.draw();

        // Swap windows
        window.swap();

        // Display framerate info
        if (audio_player.playing()) {
            frame_count++;
            current_time = audio_player.current_time();
            current_time_str = audio_player.current_time_str();
            if ((current_time - start_time) >= 2 || force_refresh) {
                printf("\r%s (fps: %4.4f)", current_time_str.c_str(),
                       frame_count / (current_time - start_time));
                fflush(stdout);
                frame_count = 0;
                start_time = current_time;
                force_refresh = false;
            }
        }
    }

    // Cleanup
    audio_player.pause();
    std::cout << endl;

    return EXIT_SUCCESS;
}
