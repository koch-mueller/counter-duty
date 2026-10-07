#version 400

layout(location = 0) in vec4 VertexPos;
layout(location = 1) in vec4 VertexNormal;
layout(location = 2) in vec2 VertexTexcoord;
layout(location = 3) in vec3 VertexTangente;
layout(location = 4) in vec3 VertexBitangente;

out vec3 Position;
out vec3 Normal;
out vec2 Texcoord;

out vec3 WorldTangente;
out vec3 WorldBitangente;

uniform mat4 ModelMat;
uniform mat4 ModelViewProjMat;

void main()
{
    Position =
        (ModelMat * VertexPos).xyz;

    mat3 modelMatrix =
        mat3(ModelMat);

    mat3 normalMatrix =
        transpose(
            inverse(
                modelMatrix));

    Normal =
        normalize(
            normalMatrix *
            VertexNormal.xyz);

    WorldTangente =
        normalize(
            modelMatrix *
            VertexTangente);

    WorldBitangente =
        normalize(
            modelMatrix *
            VertexBitangente);

    Texcoord =
        VertexTexcoord;

    gl_Position =
        ModelViewProjMat *
        VertexPos;
}