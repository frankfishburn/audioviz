R"(#version 320 es
precision mediump float;

uniform sampler2D screenTexture;
out vec4 FragColor;

void main()
{

    FragColor = texelFetch(screenTexture, ivec2(gl_FragCoord.xy), 0);   
    
})"
