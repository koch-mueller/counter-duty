#version 400

uniform vec3 OutlineColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(OutlineColor, 1.0);
}