R"(#version 320 es
in vec2 position;
out vec2 TexCoords;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0, 1.0); 
    TexCoords = 0.5 * position + 0.5;
})" 