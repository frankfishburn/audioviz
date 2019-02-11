R"(

const vec2 positions[4] = vec2[4](  vec2(  1.0, -1.0),
                                    vec2( -1.0, -1.0),
                                    vec2( -1.0,  1.0),
                                    vec2(  1.0,  1.0) );

void main()
{
    vec2 position = positions[gl_VertexID];
    gl_Position = vec4(position.x, position.y, 0.0, 1.0); 
    
})" 