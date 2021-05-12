R"(
in float amplitude;
uniform int num_freq;

void main(void){

    int vertexID;

    // Obtain vertex ID corrected for triangle strip layout
    if (gl_VertexID < 2*(num_freq+1)) {

        // Left channel
        vertexID = gl_VertexID;

    } else {

        // Right channel
        vertexID = 4*num_freq - gl_VertexID;

    }

    // Convert corrected vertex ID to frequency index
    float freq = floor(float(vertexID)/2.0);

    // Normalize frequency to [-1,1]
    freq = 2.0 * (freq - float(num_freq)/2.0) / float(num_freq);

    //
    float amp = 0.5 + 0.49 * amplitude;

    // Convert to circle
    float x_pos = amp * sin(radians(180) * freq);
    float y_pos = amp * cos(radians(180) * freq);

    gl_Position = vec4( x_pos, y_pos, 0.0, 1.0);

})"
