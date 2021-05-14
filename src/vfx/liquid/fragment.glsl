R"(
precision mediump float;
uniform int min_note;
uniform int max_note;

uniform vec2 resolution;
out vec4 FragColor;

void main(void) {

    // Position on the x-axis [0, 1] and y-axis [-1, 1]
    float x_pos = gl_FragCoord.x / resolution.x;
    float y_pos = 2.0 * gl_FragCoord.y / resolution.y - 1.0;

    // Determine note for this position
    float range = max_note - min_note;
    float note_idx = x_pos * range - range / 2.0;
    float note = mod(note_idx, 12.0);

    // Apply color of nearest note
    int nearest = int(round(note));
    if (nearest == 0) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else if (nearest == 1) {
        FragColor = vec4(1.0, 0.53272112, 0.0, 1.0);
    } else if (nearest == 2) {
        FragColor = vec4(0.93455776, 1.0, 0.0, 1.0);
    } else if (nearest == 3) {
        FragColor = vec4(0.40183664, 1.0, 0.0, 1.0);
    } else if (nearest == 4) {
        FragColor = vec4(0.0, 1.0, 0.15404569, 1.0);
    } else if (nearest == 5) {
        FragColor = vec4(0.0, 1.0, 0.68676346, 1.0);
    } else if (nearest == 6) {
        FragColor = vec4(0.0, 0.78051739, 1.0, 1.0);
    } else if (nearest == 7) {
        FragColor = vec4(0.0, 0.24779627, 1.0, 1.0);
    } else if (nearest == 8) {
        FragColor = vec4(0.30808664, 0.0, 1.0, 1.0);
    } else if (nearest == 9) {
        FragColor = vec4(0.84080776, 0.0, 1.0, 1.0);
    } else if (nearest == 10) {
        FragColor = vec4(1.0, 0.0, 0.62647112, 1.0);
    } else {
        FragColor = vec4(1.0, 0.0, 0.09375000, 1.0);
    }

    // Blend color with white if amplitude near zero
    float dist = abs(y_pos);
    if (dist < .05) {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    } else if (dist < .5) {
        float color_weight = (dist - .05) / .45;
        FragColor = FragColor * color_weight + (1.0 - color_weight);
    }

    // Mask out anything too far from a pure note
    float purity = 1.0 - 2.0 * abs(note - round(note));
    if (purity < .75) {
        FragColor = vec4( 0.0, 0.0, 0.0, 1.0 );
    }

}
)"
