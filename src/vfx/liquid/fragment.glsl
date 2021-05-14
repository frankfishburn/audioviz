R"(
precision mediump float;

uniform vec2 resolution;
out vec4 FragColor;

void main(void) {

    float x_pos = gl_FragCoord.x / resolution.x;

    // Apply the jet colormap
    float R, G, B;
    R = 4.0*x_pos - 2.0;
    if (x_pos<0.5) {
        G = 4.0*x_pos;
    } else {
        G = -4.0*x_pos + 4.0;
    }
    B = -4.0*x_pos + 2.0;

    FragColor = vec4( R, G, B, 1.0 );
}
)"
