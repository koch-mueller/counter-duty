#version 400

layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in vec3 VertexTextureCoordinate;

uniform vec3 ScreenPosition;
uniform vec3 CharacterSize;
uniform vec3 ScreenSize;

uniform vec3 UVMin;
uniform vec3 UVMax;

out vec2 TextureCoordinate;

void main()
{
    vec2 pixelPosition = ScreenPosition.xy + VertexPosition.xy * CharacterSize.xy;

    vec2 normalizedPosition;

    normalizedPosition.x = pixelPosition.x / ScreenSize.x * 2.0 - 1.0;

    normalizedPosition.y = 1.0 - pixelPosition.y / ScreenSize.y * 2.0;

    gl_Position = vec4(normalizedPosition, 0.0, 1.0);

    TextureCoordinate = mix(UVMin.xy, UVMax.xy, VertexTextureCoordinate.xy);
}