R"(#version 320 es
precision mediump float;

const float gaussian_weights[] = float[] (0.2270270270,
                                          0.3162162162,
                                          0.0702702703);

uniform float     blur_radius;
uniform sampler2D texture_sampler;

in vec2 TexCoords;
out vec4 output_color;

void main()
{
    vec4  total_color      = vec4(0.0);
    float image_resolution = float((textureSize(texture_sampler, 0)).x);
    float blur_step        = blur_radius / image_resolution;
    /* Calculate blurred colour. */
    /* Blur a texel on the right. */
    total_color = texture(texture_sampler, vec2(TexCoords.x + 1.0 * blur_step, TexCoords.y)) * gaussian_weights[0] +
                  texture(texture_sampler, vec2(TexCoords.x + 2.0 * blur_step, TexCoords.y)) * gaussian_weights[1] +
                  texture(texture_sampler, vec2(TexCoords.x + 3.0 * blur_step, TexCoords.y)) * gaussian_weights[2];
    /* Blur a texel on the left. */
    total_color += texture(texture_sampler, vec2(TexCoords.x - 1.0 * blur_step, TexCoords.y)) * gaussian_weights[0] +
                   texture(texture_sampler, vec2(TexCoords.x - 2.0 * blur_step, TexCoords.y)) * gaussian_weights[1] +
                   texture(texture_sampler, vec2(TexCoords.x - 3.0 * blur_step, TexCoords.y)) * gaussian_weights[2];
    /* Set the output colour. */
    output_color = vec4(total_color.xyz, 1.0);
};)"
