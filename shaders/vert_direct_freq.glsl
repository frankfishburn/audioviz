R"(#version 320 es
in float amplitude;
uniform int num_freq;
uniform float multiplier;

void main(void){
 
    float freq = floor(float(gl_VertexID)/2.0);
    float y_pos = 1.95 * (freq + 0.5 - float(num_freq)/2.0) / float(num_freq);
    float x_pos = multiplier * 0.99 * min(amplitude,1.0);
    
    gl_Position = vec4( x_pos, y_pos, 0.0, 1.0);

})"