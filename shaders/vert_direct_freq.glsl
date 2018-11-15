R"(#version 320 es
in float amplitude;
uniform int num_freq;
uniform int num_time;
uniform int time_idx;
uniform float multiplier;

void main(void){
 
    float y_height = 1.0 / float(num_time);
    float y_pos = 2.0 * float(time_idx) * y_height + y_height - 1.0;

    float x_pos = 1.95 * (float(gl_VertexID) - float(num_freq)/2.0) / float(num_freq);


    gl_Position = vec4( x_pos, multiplier * 0.99 * y_height * min(amplitude,1.0) + y_pos, 1.0, 1.0);

})"