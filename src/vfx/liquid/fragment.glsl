R"(
precision mediump float;

uniform vec2 resolution;
out vec4 FragColor;

void main(void) {

    float ypos = gl_FragCoord.y / resolution.y;

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
