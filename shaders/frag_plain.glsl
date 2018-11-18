R"(#version 320 es
precision mediump float;

uniform vec2 resolution;
out vec4 FragColor;

void main(void) {

    float xpos = 2.0 * abs(gl_FragCoord.x - resolution.x/2.0) / resolution.x; 
    float ypos = 2.0 * abs(gl_FragCoord.y - resolution.y/2.0) / resolution.y;

    FragColor = vec4( 3.0*ypos , xpos , 1.0 , 1.0 );

}
)"