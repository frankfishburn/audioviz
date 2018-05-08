R"(#version 130
precision mediump float;
uniform vec3 RGB;
out vec4 FragColor;

void main(void) {
    FragColor = vec4(RGB.x, RGB.y, RGB.z, 1.0 );
})"