R"(#version 320 es
precision mediump float;

uniform vec2 resolution;
out vec4 FragColor;

void main(void) {

    float xpos = 2.0 * abs(gl_FragCoord.x - resolution.x/2.0) / resolution.x; 
    float ypos = abs(gl_FragCoord.y) / resolution.y;

    // Apply the jet colormap
    float R, G, B;
    R = 4.0*ypos - 2.0;
    if (ypos<0.5) {
        G = 4.0*ypos;
    } else {
        G = -4.0*ypos + 4.0;
    }
    B = -4.0*ypos + 2.0;

    FragColor = vec4( R, G, B, 1.0 );
}
)"