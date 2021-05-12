R"(
precision mediump float;

uniform vec2 resolution;
out vec4 FragColor;

void main(void) {

    // Coordinates in normalized space
    float xpos = 2.0 * gl_FragCoord.x / resolution.x - 1.0;
    float ypos = 2.0 * gl_FragCoord.y / resolution.y - 1.0;

    // Distance from center
    float dist = sqrt(xpos*xpos + ypos*ypos);

    // Adjust range
    dist = 2.0 * (dist - 0.50);

    // Apply the jet colormap
    float R, G, B;
    R = 4.0*dist - 2.0;
    if (dist<0.5) {
        G = 4.0*dist;
    } else {
        G = -4.0*dist + 4.0;
    }
    B = -4.0*dist + 2.0;

    FragColor = vec4( R, G, B, 1.0 );
}
)"
