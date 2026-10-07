
#include "PhongMaterial.h"

#include "AssetLoader.h"

#include "../engine/PhongShader.h"
#include "../engine/Texture.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

namespace
{
    std::string trim(const std::string& value)
    {
        std::size_t first = value.find_first_not_of(" \t\r\n");

        if (first == std::string::npos)
        {
            return "";
        }

        std::size_t last = value.find_last_not_of(" \t\r\n");

        return value.substr(first, last - first + 1);
    }

    std::string lowerCase(std::string value)
    {
        std::transform(value.begin(),
                       value.end(),
                       value.begin(),
                       [](unsigned char character)
                       {
                           return static_cast<char>(std::tolower(character));
                       });

        return value;
    }

    std::string directoryPath(const std::string& filePath)
    {
        std::size_t separator = filePath.find_last_of("/\\");

        if (separator == std::string::npos)
        {
            return "";
        }

        return filePath.substr(0, separator + 1);
    }

    std::string readTextureName(std::istringstream& stream)
    {
        std::string token;
        std::string textureName;

        while (stream >> token)
        {
            textureName = token;
        }

        std::replace(textureName.begin(), textureName.end(), '\\', '/');

        return textureName;
    }
}

PhongMaterial::PhongMaterial()
    : m_ambientColor(),
      m_diffuseColor(),
      m_specularColor(),
      m_specularExp(10.0f),
      m_diffuseTexture(nullptr),
      m_normalTexture(nullptr)
{
    resetValues();
}

PhongMaterial::~PhongMaterial()
{
    releaseTextures();
}

bool PhongMaterial::load(const AssetLoader& assetLoader, const std::string& relativePath)
{
    releaseTextures();

    resetValues();

    std::string fullPath = assetLoader.assetPath(relativePath);

    std::ifstream file(fullPath);

    if (!file.is_open())
    {
        std::cerr << "Failed to load material: " << fullPath << "\n";

        return false;
    }

    std::string materialDirectory = directoryPath(fullPath);

    // Für die Projektmaterialien wird jeweils der erste Materialblock der MTL-Datei verwendet
    bool materialFound = false;

    std::string line;

    while (std::getline(file, line))
    {
        std::size_t commentPosition = line.find('#');

        if (commentPosition != std::string::npos)
        {
            line.erase(commentPosition);
        }

        line = trim(line);

        if (line.empty())
        {
            continue;
        }

        std::istringstream stream(line);

        std::string key;

        stream >> key;

        key = lowerCase(key);

        // Die wenigen benötigten MTL-Eigenschaften werden direkt eingelesen, dadurch bleibt die Materialkonfiguration unabhängig vom Model-Loader
        if (key == "newmtl")
        {
            if (materialFound)
            {
                break;
            }

            materialFound = true;

            continue;
        }

        if (!materialFound)
        {
            continue;
        }

        if (key == "ka")
        {
            stream >> m_ambientColor.R >> m_ambientColor.G >> m_ambientColor.B;
        }
        else if (key == "kd")
        {
            stream >> m_diffuseColor.R >> m_diffuseColor.G >> m_diffuseColor.B;
        }
        else if (key == "ks")
        {
            stream >> m_specularColor.R >> m_specularColor.G >> m_specularColor.B;
        }
        else if (key == "ns")
        {
            stream >> m_specularExp;
        }
        else if (key == "map_kd")
        {
            std::string textureName = readTextureName(stream);

            if (!textureName.empty())
            {
                std::string texturePath = materialDirectory + textureName;

                // Shared Textures verhindern, dass identische Materialtexturen mehrfach geladen werden
                m_diffuseTexture = Texture::LoadShared(texturePath.c_str());
            }
        }
        else if (key == "map_bump" || key == "bump" || key == "norm")
        {
            std::string textureName = readTextureName(stream);

            if (!textureName.empty())
            {
                std::string texturePath = materialDirectory + textureName;

                m_normalTexture = Texture::LoadShared(texturePath.c_str());
            }
        }
    }

    return materialFound;
}

PhongShader* PhongMaterial::createShader() const
{
    PhongShader* shader = new PhongShader();

    shader->ambientColor(m_ambientColor);

    shader->diffuseColor(m_diffuseColor);

    shader->specularColor(m_specularColor);

    shader->specularExp(m_specularExp);

    shader->diffuseTexture(m_diffuseTexture);

    shader->normalTexture(m_normalTexture);

    return shader;
}

void PhongMaterial::resetValues()
{
    m_ambientColor = Color(0.0f, 0.0f, 0.0f);

    m_diffuseColor = Color(1.0f, 1.0f, 1.0f);

    m_specularColor = Color(0.3f, 0.3f, 0.3f);

    m_specularExp = 10.0f;
}

void PhongMaterial::releaseTextures()
{
    if (m_diffuseTexture != nullptr)
    {
        Texture::ReleaseShared(m_diffuseTexture);

        m_diffuseTexture = nullptr;
    }

    if (m_normalTexture != nullptr)
    {
        Texture::ReleaseShared(m_normalTexture);

        m_normalTexture = nullptr;
    }
}
