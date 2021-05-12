#ifndef ECLIPSE_H
#define ECLIPSE_H

#include <string>
#include <vector>

#include "algorithm/stft.h"
#include "audio/i_source.h"
#include "video/framebuffer.h"
#include "video/shader_program.h"

class FXEclipse {
   public:
    FXEclipse(const IAudioSource&, const FrameBuffer&);
    ~FXEclipse();
    void draw(const unsigned long);

    std::string name();
    void set_resolution(const float width, const float height);
    void set_resolution(const int width, const int height);
    void print_errors() const { shader_->print_errors(); };

   private:
    const IAudioSource& audio_source_;
    const STFT stft_;
    const FrameBuffer& fb_;

    int num_vertices_;
    std::vector<float> vertices_;
    ShaderProgram* shader_;
    GLuint VBO_;
    GLuint VAO_;
};

#endif /* ECLIPSE_H */
