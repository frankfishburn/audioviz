R"(#version 320 es
precision mediump float;
precision highp sampler2DMS;

const float gaussian_weights[] = float[] (0.2270270270,
                                          0.3162162162,
                                          0.0702702703);

uniform int         num_samples;
uniform sampler2DMS texture_sampler;

out vec4 output_color;

void main()
{
    vec4  total_color      = vec4(0.0);
    
    for (int i=0; i<num_samples; i++) {

        total_color += texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y + 1.0), i) * gaussian_weights[0] +
                       texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y + 2.0), i) * gaussian_weights[1] +
                       texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y + 3.0), i) * gaussian_weights[2];

        total_color += texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y - 1.0), i) * gaussian_weights[0] +
                       texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y - 2.0), i) * gaussian_weights[1] +
                       texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y - 3.0), i) * gaussian_weights[2];

    }
    total_color /= float(num_samples);
    
    output_color = vec4(total_color.xyz, 1.0);

};)"
