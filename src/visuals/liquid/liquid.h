#ifndef LIQUID_H
#define LIQUID_H

#include <string>
#include <vector>

#include "algorithm/stft.h"
#include "audio/i_source.h"
#include "video/framebuffer.h"
#include "video/shader_program.h"
#include "visuals/i_visual.h"

class LiquidVisual : public IVisual {
   public:
    LiquidVisual(const IAudioSource&, const FrameBuffer&);
    void draw(const unsigned long) override;

    std::string name() override;
    void set_resolution(const float width, const float height) override;
    void set_resolution(const int width, const int height) override;

   private:
    const IAudioSource& audio_source_;
    const STFT stft_;
    const FrameBuffer& fb_;

    int num_vertices_;
    std::vector<float> vertices_;
    ShaderProgram program_;
    GLuint VBO_;
    GLuint VAO_;
};

#endif /* LIQUID_H */
