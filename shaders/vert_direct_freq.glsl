R"(#version 320 es
in float amplitude;
uniform int num_freq;
uniform float multiplier;

void main(void){
 
    float trapezoidID = floor(float(gl_VertexID) / 6.0);
    float trapvertexID = float(gl_VertexID) - 6.0*trapezoidID;

    if (trapvertexID==0.0 || trapvertexID==1.0 || trapvertexID==4.0) {
        trapezoidID -= 0.5;
    } else {
        trapezoidID += 0.5;
    }
    
    float y_pos = 1.95 * (trapezoidID + 0.5 - float(num_freq)/2.0) / float(num_freq);
    float x_pos = multiplier * 0.99 * min(amplitude,1.0);
    
    gl_Position = vec4( x_pos, y_pos, 0.0, 1.0);

})"