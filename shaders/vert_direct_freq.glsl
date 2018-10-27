R"(#version 320 es
in float amplitude;
uniform int num_freq;
uniform float multiplier;

void main(void){

    float x_pos = 1.95 * (float(gl_VertexID) - float(num_freq)/2.0) / float(num_freq);

    gl_Position = vec4(x_pos, multiplier*amplitude*0.99, 1.0, 1.0);

})"