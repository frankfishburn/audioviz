R"(#version 320 es
in float amplitude;
uniform int num_freq;
uniform float multiplier;

void main(void){
 
    float freq = floor(float(gl_VertexID) / 6.0);
    float vert = float(gl_VertexID) - 6.0*freq;
    if (vert==2.0 || vert==4.0 || vert==5.0) {
        freq += 0.2;
    } else {
        freq -= 0.2;
    }
    
    float y_pos = 1.95 * (freq + 0.5 - float(num_freq)/2.0) / float(num_freq);
    float x_pos = multiplier * 0.99 * min(amplitude,1.0);
    
    gl_Position = vec4( x_pos, y_pos, 0.0, 1.0);

})"