#version 400

uniform sampler2D FontAtlas;
uniform vec3 TextColor;

in vec2 TextureCoordinate;

out vec4 FragmentColor;

void main()
{
    vec4 atlasColor = texture(FontAtlas, TextureCoordinate);

    FragmentColor = vec4(TextColor * atlasColor.rgb, atlasColor.a);
}