R"(#version 320 es
in float amplitude;
uniform int num_freq;
uniform float nyquist_freq;

void main(void){

    float freq = float(gl_VertexID) * nyquist_freq / float(num_freq);
    float x_pos = 1.95 * (float(gl_VertexID) - float(num_freq)/2.0) / float(num_freq);

    gl_Position = vec4(x_pos, 2.0*amplitude - 0.99, 1.0, 1.0);

})"