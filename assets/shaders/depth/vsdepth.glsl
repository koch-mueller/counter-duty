#version 400
layout(location=0) in vec4 VertexPos;
out vec4 Position;

uniform mat4 ModelMat;
uniform mat4 ModelViewProjMat;
void main()
{
    Position = ModelViewProjMat * VertexPos;
    gl_Position = Position;
}

