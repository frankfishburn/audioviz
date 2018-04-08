#version 130

attribute float position;
uniform float xScale;

void main(void){

    //gl_Position = vec4(position.x, position.y, 1.0, 1.0);
    
    float xPos = gl_VertexID * xScale - 1;
    gl_Position = vec4(xPos, position, 1.0, 1.0);

}