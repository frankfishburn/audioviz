R"(#version 320 es
precision mediump float;

const float gaussian_weights[] = float[] (0.2270270270,
                                          0.3162162162,
                                          0.0702702703);

uniform sampler2D texture_sampler;

out vec4 output_color;

void main()
{
    vec4  total_color      = vec4(0.0);

    total_color += texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y + 1.0), 0) * gaussian_weights[0] +
                   texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y + 2.0), 0) * gaussian_weights[1] +
                   texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y + 3.0), 0) * gaussian_weights[2];

    total_color += texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y - 1.0), 0) * gaussian_weights[0] +
                   texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y - 2.0), 0) * gaussian_weights[1] +
                   texelFetch(texture_sampler, ivec2(gl_FragCoord.x, gl_FragCoord.y - 3.0), 0) * gaussian_weights[2];
    
    output_color = vec4(total_color.xyz, 1.0);

};)"
