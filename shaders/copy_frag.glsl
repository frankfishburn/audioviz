R"(#version 320 es
precision mediump float;
precision highp sampler2DMS;

uniform sampler2DMS screenTexture;
uniform int num_samples;
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0);
    for (int i=0; i<num_samples; i++) {
        FragColor += texelFetch(screenTexture, ivec2(gl_FragCoord.xy), i);
    }
    FragColor /= float(num_samples);
    
    
})"
