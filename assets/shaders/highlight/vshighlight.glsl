#version 400

layout(location = 0) in vec4 VertexPos;

uniform mat4 ModelViewProjMat;
uniform float OutlineScale;

void main()
{
    vec4 expandedPosition =
        vec4(
            VertexPos.xyz * OutlineScale,
            1.0);

    gl_Position =
        ModelViewProjMat *
        expandedPosition;
}