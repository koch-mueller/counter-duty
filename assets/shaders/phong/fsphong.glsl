#version 400

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;

in vec3 WorldTangente;
in vec3 WorldBitangente;

out vec4 FragColor;

uniform vec3 EyePos;

uniform vec3 DiffuseColor;
uniform vec3 SpecularColor;
uniform vec3 AmbientColor;
uniform float SpecularExp;

uniform sampler2D DiffuseTexture;
uniform sampler2D OpacityTex;
uniform sampler2D NormalTexture;
uniform float AlphaMultiplier;

uniform bool UseOpacityTexture;

const int MAX_LIGHTS = 14;

struct Light
{
    int Type;
    vec3 Color;
    vec3 Position;
    vec3 Direction;
    vec3 Attenuation;
    vec3 SpotRadius;
    int ShadowIndex;
};

uniform Lights
{
    int LightCount;
    Light lights[MAX_LIGHTS];
};

float sat(float value)
{
    return clamp(
        value,
        0.0,
        1.0);
}

void main()
{
    vec4 DiffTex =
    texture(
        DiffuseTexture,
        Texcoord);

if (UseOpacityTexture)
{
    float rawOpacity =
        texture(
            OpacityTex,
            Texcoord).r;

    float cleanedOpacity =
        smoothstep(
            0.30,
            0.80,
            rawOpacity);

    float borderDistance =
        min(
            min(
                Texcoord.x,
                1.0 - Texcoord.x),
            min(
                Texcoord.y,
                1.0 - Texcoord.y));

    float borderFade =
        smoothstep(
            0.0,
            0.14,
            borderDistance);

    DiffTex.a =
        cleanedOpacity *
        borderFade;
}

if (DiffTex.a < 0.12)
{
    discard;
}

    vec3 N =
        normalize(
            Normal);

    vec3 E =
        normalize(
            EyePos -
            Position);

    vec3 Color =
        AmbientColor *
        DiffTex.rgb;

    float SafeSpecularExp =
        max(
            SpecularExp,
            1.0);

    for (int i = 0; i < LightCount; ++i)
    {
        if (lights[i].Type == 0)
        {
            float Dist =
                length(
                    lights[i].Position -
                    Position);

            float Intensity =
                1.0 /
                (
                    lights[i].Attenuation.x +
                    lights[i].Attenuation.y * Dist +
                    lights[i].Attenuation.z * Dist * Dist
                );

            vec3 L =
                normalize(
                    lights[i].Position -
                    Position);

            vec3 H =
                normalize(
                    E + L);

            vec3 DiffuseComponent =
                lights[i].Color *
                DiffuseColor *
                sat(
                    dot(
                        N,
                        L));

            vec3 SpecularComponent =
                lights[i].Color *
                SpecularColor *
                pow(
                    sat(
                        dot(
                            N,
                            H)),
                    SafeSpecularExp);

            Color +=
                Intensity *
                (
                    DiffuseComponent *
                    DiffTex.rgb +
                    SpecularComponent
                );
        }
        else if (lights[i].Type == 1)
        {
            vec3 L =
                normalize(
                    -lights[i].Direction);

            vec3 H =
                normalize(
                    E + L);

            vec3 DiffuseComponent =
                lights[i].Color *
                DiffuseColor *
                sat(
                    dot(
                        N,
                        L));

            vec3 SpecularComponent =
                lights[i].Color *
                SpecularColor *
                pow(
                    sat(
                        dot(
                            N,
                            H)),
                    SafeSpecularExp);

            Color +=
                DiffuseComponent *
                DiffTex.rgb +
                SpecularComponent;
        }
        else if (lights[i].Type == 2)
        {
            float Dist =
                length(
                    lights[i].Position -
                    Position);

            float Intensity =
                1.0 /
                (
                    lights[i].Attenuation.x +
                    lights[i].Attenuation.y * Dist +
                    lights[i].Attenuation.z * Dist * Dist
                );

            vec3 L =
                normalize(
                    lights[i].Position -
                    Position);

            vec3 H =
                normalize(
                    E + L);

            vec3 DiffuseComponent =
                lights[i].Color *
                DiffuseColor *
                sat(
                    dot(
                        N,
                        L));

            vec3 SpecularComponent =
                lights[i].Color *
                SpecularColor *
                pow(
                    sat(
                        dot(
                            N,
                            H)),
                    SafeSpecularExp);

            float angle =
                acos(
                    clamp(
                        dot(
                            normalize(
                                Position -
                                lights[i].Position),
                            normalize(
                                lights[i].Direction)),
                        -1.0,
                        1.0));

            float spotFactor =
                1.0 -
                sat(
                    (
                        angle -
                        lights[i].SpotRadius.x
                    ) /
                    (
                        lights[i].SpotRadius.y -
                        lights[i].SpotRadius.x
                    ));

            Color +=
                Intensity *
                (
                    DiffuseComponent *
                    DiffTex.rgb +
                    SpecularComponent
                ) *
                spotFactor;
        }
    }

    FragColor =
    vec4(
        Color,
        DiffTex.a * AlphaMultiplier);
}