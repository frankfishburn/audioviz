#version 130

attribute float position;
uniform float sample_rate;
uniform float current_time;
uniform float window_duration;

void main(void){

    // Rescale vertex index to seconds
    float xTime = gl_VertexID / sample_rate;

    // Center on current time and scale to time window
    float xPos = (xTime - current_time) / window_duration;

    gl_Position = vec4(xPos, position, 1.0, 1.0);

}