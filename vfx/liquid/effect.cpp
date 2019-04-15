#include <vector>

#include "effect.h"

const char* src_shader_vertex =
#include "vfx/liquid/vertex.glsl"
    ;
const char* src_shader_fragment =
#include "vfx/liquid/fragment.glsl"
    ;

FXLiquid::FXLiquid(STFT* stft_ptr, FrameBuffer* fb_ptr) {
    // Set shader name and program structures
    effect_name = "Liquid";
    stft        = stft_ptr;
    fb          = fb_ptr;

    // Compile and link shader
    shader = new ShaderProgram(src_shader_vertex, src_shader_fragment);
    // shader->use();

    // Set data parameters and allocate
    num_frequencies = stft->maxGoodFreq();
    num_vertices    = 4 * num_frequencies;
    vertices.resize(num_vertices);

    // Set static uniforms
    shader->set_uniform("num_freq", (int)num_frequencies);
    shader->set_uniform("resolution", (float)fb->width(), (float)fb->height());

    // Set up vertex buffer/array object for each channel
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

    glBindVertexArray(VAO);
    shader->set_attrib("amplitude", 1);
}

void FXLiquid::draw() {
    float* power_left;
    float* power_right;

    power_left = stft->getSpectrum(0).power.data();

    if (stft->numChannels() == 1) {
        // Mono
        power_right = power_left;
    } else {
        // Stereo
        power_right = stft->getSpectrum(1).power.data();
    }

    // Copy left channel power spectrum into vertex buffer
    for (int freq = 0; freq < num_frequencies; freq++) {
        vertices[2 * freq + 1] = -power_left[freq];
    }

    // Copy right channel power spectrum into vertex buffer
    for (int freq = 0; freq < num_frequencies; freq++) {
        vertices[num_vertices - 2 * freq] = power_right[freq];
    }

    // Update the buffer
    fb->bind();
    shader->use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, num_vertices * sizeof(GLfloat), vertices.data());
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices);
    fb->unbind();
}

std::string FXLiquid::name() {
    return effect_name;
}

void FXLiquid::set_resolution(const float width, const float height) {
    shader->set_uniform("resolution", width, height);
}

void FXLiquid::set_resolution(const int width, const int height) {
    shader->set_uniform("resolution", (float)width, (float)height);
}

FXLiquid::~FXLiquid() {
    delete shader;
}