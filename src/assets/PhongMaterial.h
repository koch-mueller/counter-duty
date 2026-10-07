#pragma once

#include "../engine/color.h"

#include <string>

class AssetLoader;
class PhongShader;
class Texture;

// Hält Materialparameter und Texturen aus einer MTL-Datei und überträgt sie auf den Phong-Shader
class PhongMaterial
{
public:
    PhongMaterial();

    ~PhongMaterial();

    PhongMaterial(const PhongMaterial&) = delete;

    PhongMaterial& operator=(const PhongMaterial&) = delete;

    bool load(const AssetLoader& assetLoader, const std::string& relativePath);

    PhongShader* createShader() const;

private:
    void resetValues();

    void releaseTextures();

    Color m_ambientColor;
    Color m_diffuseColor;
    Color m_specularColor;

    float m_specularExp;

    const Texture* m_diffuseTexture;
    const Texture* m_normalTexture;
};
