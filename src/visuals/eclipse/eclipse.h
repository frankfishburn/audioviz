#ifndef ECLIPSE_H
#define ECLIPSE_H

#include <string>
#include <vector>

#include "algorithm/stft.h"
#include "audio/i_source.h"
#include "video/framebuffer.h"
#include "video/shader_program.h"
#include "video/vertex_buffer.h"
#include "visuals/i_visual.h"

class EclipseVisual : public IVisual {
   public:
    EclipseVisual(const IAudioSource&, const FrameBuffer&);
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
    VertexBuffer vertex_buffer_;
};

#endif /* ECLIPSE_H */
