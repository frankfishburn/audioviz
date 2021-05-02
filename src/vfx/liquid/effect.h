#ifndef LIQUID_H
#define LIQUID_H

#include <string>
#include <vector>

#include "algorithm/stft.h"
#include "video/framebuffer.h"
#include "video/shader_program.h"

class FXLiquid {
   public:
    FXLiquid(STFT *, FrameBuffer *);
    ~FXLiquid();
    void draw();

    std::string name();
    void set_resolution(const float width, const float height);
    void set_resolution(const int width, const int height);

   private:
    STFT *stft;
    FrameBuffer *fb;
    std::string effect_name;
    int num_frequencies;
    int num_vertices;
    float *frequencies;
    std::vector<float> vertices;
    ShaderProgram *shader;
    GLuint VBO;
    GLuint VAO;
};

#endif /* LIQUID_H */
