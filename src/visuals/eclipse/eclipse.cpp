#include "eclipse.h"

#include <vector>

static const char* src_shader_vertex =
#include "visuals/eclipse/vertex.glsl"
    ;
static const char* src_shader_fragment =
#include "visuals/eclipse/fragment.glsl"
    ;

static constexpr unsigned long segment_length = 16384;
static constexpr unsigned long window_length = segment_length;
static constexpr unsigned long window_overlap = 0;
static constexpr unsigned long transform_length = 4 * window_length;

static STFT create_stft(const IAudioSource& audio_source) {
    SpectrogramInput props;
    props.data_size = sizeof(float);
    props.sample_rate = audio_source.sample_rate();
    props.num_samples = segment_length;
    props.stride = 1;  // Not equal to # of channels since we deinterleave first

    SpectrogramConfig config;
    config.padding_mode = PAD;
    config.window_length = window_length;
    config.window_overlap = window_overlap;
    config.transform_length = transform_length;
    config.window_type = HAMMING;

    return STFT(props, config);
}

EclipseVisual::EclipseVisual(const IAudioSource& audio_source,
                             const FrameBuffer& fb)
    : audio_source_(audio_source),
      stft_(create_stft(audio_source)),
      fb_(fb),
      vertex_buffer_(VertexBuffer(2 * stft_.length())) {
    // Set data parameters and allocate
    num_vertices_ = 2 * stft_.length();
    vertices_.resize(num_vertices_);

    // Compile and link shader
    program_.compile(src_shader_vertex, src_shader_fragment);

    // Set uniforms and array
    program_.set_uniform("num_freq", (int)stft_.length());
    program_.set_uniform("min_note", -50);
    program_.set_uniform("max_note", 50);
    program_.set_uniform("resolution", (float)fb_.width(), (float)fb_.height());
    program_.set_input("amplitude", vertex_buffer_);
}

void EclipseVisual::draw(const unsigned long position) {
    // Get signal at current position
    const unsigned long center = position - segment_length / 2;
    std::vector<float> signal_left =
        audio_source_.get_segment(0, center, segment_length);
    std::vector<float> signal_right =
        audio_source_.get_segment(1, center, segment_length);

    // Must check size since get_window can return a shorter vector than
    // requested
    if (signal_left.size() == segment_length &&
        signal_right.size() == segment_length) {
        // Compute spectrum
        std::vector<float> power_left = stft_.compute(signal_left);
        std::vector<float> power_right = stft_.compute(signal_right);

        // Arrange spectra in vertex array
        for (unsigned long idx = 0; idx < stft_.length(); idx++)
            vertices_[idx] = power_left[idx];
        for (unsigned long idx = 0; idx < stft_.length(); idx++)
            vertices_[num_vertices_ - idx - 1] = power_right[idx];

    } else {
        // Clear out vertex array
        for (unsigned long idx = 0; idx < vertices_.size(); idx++)
            vertices_[idx] = 0;
    }

    // Update the data
    vertex_buffer_.push(vertices_.data());

    // Draw
    fb_.bind();
    program_.use();
    glDrawArrays(GL_LINE_LOOP, 0, num_vertices_);
    fb_.unbind();
}

std::string EclipseVisual::name() { return std::string("Eclipse"); }

void EclipseVisual::set_resolution(const float width, const float height) {
    program_.set_uniform("resolution", width, height);
}

void EclipseVisual::set_resolution(const int width, const int height) {
    program_.set_uniform("resolution", (float)width, (float)height);
}
