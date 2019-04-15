#include <cstdlib>
#include <exception>
#include <functional>

#include <iostream>
#include <memory>
#include <vector>

#include "audio_manager.h"
#include "framebuffer.h"
#include "shader_program.h"
#include "stft.h"
#include "window.h"

#include "vfx/liquid/effect.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Please supply a filename to a music file.\n");
        return EXIT_FAILURE;
    }

    // Load audio and setup playback
    audio_manager audio(argv[1]);

    if (!audio.is_playable() || audio.get_num_samples() == 0) {
        fprintf(stderr, "Audio problem, bailing out!\n");
        fflush(stderr);
        return 1;
    }

    audio.print();

    // Configure spectrogram transform
    SpectrogramConfig config;
    config.padding_mode     = PAD;
    config.window_length    = 4096;
    config.window_overlap   = config.window_length * .5;
    config.transform_length = config.window_length * 4;
    config.window_type      = HAMMING;

    // Create STFT object
    STFT stft(audio, config, 4096 * 2);

    // Analyze for maximum power
    stft.analyze();

    // Initialize window and context
    Window wnd;

    // Create a framebuffer
    FrameBuffer fb(&wnd, true);

    // Setup visual effect renderer
    FXLiquid vfx(&stft, &fb);

    // Enable v-sync
    SDL_GL_SetSwapInterval(1);

    // Disable depth, enable alpha blending
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(false);

    // Check for GL errors
    wnd.check_errors();

    // Start audio
    audio.play();
    double        start_time   = 0;
    double        current_time = 0;
    std::string   current_time_str;
    unsigned long frame_count   = 0;
    bool          force_refresh = true;

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
                            audio.toggle_playback();
                            break;
                        case SDLK_LEFT:
                            audio.back();
                            start_time    = audio.get_current_time();
                            frame_count   = 0;
                            force_refresh = true;
                            break;
                        case SDLK_RIGHT:
                            audio.forward();
                            start_time    = audio.get_current_time();
                            frame_count   = 0;
                            force_refresh = true;
                            break;
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                        case SDLK_q:
                            quit = true;
                            break;
                        case SDLK_f:
                            wnd.toggle_fullscreen();
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

        // Compute current STFT
        long current_sample = audio.get_current_sample();
        stft.compute(current_sample);

        // Render visual effects to framebuffer
        vfx.draw();

        // Draw framebuffer to screen
        fb.draw();

        // Swap windows
        wnd.swap();

        // Display framerate info
        if (audio.is_playing()) {
            frame_count++;
            current_time     = audio.get_current_time();
            current_time_str = audio.get_current_time_str();
            if ((current_time - start_time) >= 2 || force_refresh) {
                printf("\r%s (fps: %4.4f)", current_time_str.c_str(), frame_count / (current_time - start_time));
                fflush(stdout);
                frame_count   = 0;
                start_time    = current_time;
                force_refresh = false;
            }
        }
    }

    // Cleanup
    audio.pause();
    printf("\r\n");

    return 0;
}
