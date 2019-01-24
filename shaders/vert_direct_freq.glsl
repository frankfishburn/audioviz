R"(#version 320 es
in float amplitude;
uniform int num_freq;

void main(void){
 
    int vertexID;

    if (gl_VertexID < 2*(num_freq+1)) {

        // Left channel
        vertexID = gl_VertexID;

    } else {
        
        // Right channel
        vertexID = 4*num_freq - gl_VertexID;
        
    }
    
    float freq = floor(float(vertexID)/2.0);
    float y_pos = 1.95 * (freq - float(num_freq)/2.0) / float(num_freq);
    float x_pos = 0.99 * min(amplitude,1.0);
    
    gl_Position = vec4( x_pos, y_pos, 0.0, 1.0);

})"