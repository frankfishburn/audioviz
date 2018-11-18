R"(#version 320 es
precision mediump float;

uniform vec2 resolution;
out vec4 FragColor;

void main(void) {

    float xpos = 2.0 * abs(gl_FragCoord.x - resolution.x/2.0) / resolution.x; 
    float ypos = 2.0 * abs(gl_FragCoord.y - resolution.y/2.0) / resolution.y;

    // Apply the jet colormap
    float R, G, B;
    R = 4.0*xpos - 1.5;
    if (xpos<0.5) {
        G = 4.0*xpos - 0.5;
    } else {
        G = -4.0*xpos + 3.5; 
    }
    B = -4.0*xpos + 2.5;

    FragColor = vec4( R, G, B, 1.0 );    
}
)"