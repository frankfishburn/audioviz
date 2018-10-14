R"(#version 320 es
in float amplitude;
uniform int center_vertex;
uniform int num_vertices;

void main(void){

    float x_pos = 2.0 * float(gl_VertexID - center_vertex) / float(num_vertices);

    gl_Position = vec4(x_pos, amplitude, 1.0, 1.0);

})"