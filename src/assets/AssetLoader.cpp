
#include "AssetLoader.h"

#include "../engine/Aabb.h"
#include "../engine/Model.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>

#include <fstream>

namespace
{
    // Die relativen Startpfade unterscheiden sich zwischen Visual Studio und Xcode/macOS
    std::string defaultAssetDirectory()
    {
#ifdef WIN32
        return "../../assets/";
#else
        return "../assets/";
#endif
    }
}

AssetLoader::AssetLoader()
{
    setAssetDirectory(defaultAssetDirectory());
}

AssetLoader::AssetLoader(const std::string& assetDirectory)
{
    setAssetDirectory(assetDirectory);
}

std::string AssetLoader::assetPath(const std::string& relativePath) const
{
    if (relativePath.empty())
    {
        return m_assetDirectory;
    }

    std::size_t firstCharacter = 0;

    while (firstCharacter < relativePath.size()
           && (relativePath[firstCharacter] == '/' || relativePath[firstCharacter] == '\\'))
    {
        ++firstCharacter;
    }

    return m_assetDirectory + relativePath.substr(firstCharacter);
}

std::string AssetLoader::preferredAssetPath(const std::string& localRelativePath,
                                            const std::string& fallbackRelativePath) const
{
    std::string localPath = assetPath(localRelativePath);
    std::ifstream localFile(localPath, std::ios::binary);

    if (localFile.good())
    {
        return localPath;
    }

    return assetPath(fallbackRelativePath);
}

Model* AssetLoader::loadModel(const std::string& relativePath, bool fitSize) const
{
    if (relativePath.empty())
    {
        return nullptr;
    }

    std::string fullPath = assetPath(relativePath);

    Model* model = new Model();

    bool loaded = model->load(fullPath.c_str(), fitSize);

    if (!loaded)
    {
        delete model;

        return nullptr;
    }

    return model;
}

bool AssetLoader::loadModelBounds(const std::string& relativePath, AABB& bounds) const
{
    if (relativePath.empty())
    {
        return false;
    }

    std::string fullPath = assetPath(relativePath);

    unsigned int importFlags = 0;

    std::string modelPath = fullPath;

    std::transform(modelPath.begin(),
                   modelPath.end(),
                   modelPath.begin(),
                   [](unsigned char character)
                   {
                       return static_cast<char>(std::tolower(character));
                   });

    bool isGltf =
        (modelPath.size() >= 5 && modelPath.compare(modelPath.size() - 5, 5, ".gltf") == 0)
        || (modelPath.size() >= 4 && modelPath.compare(modelPath.size() - 4, 4, ".glb") == 0);

    if (isGltf)
    {
        // glTF speichert häufig zusätzliche Node-Transformationen
        // Für die Bounds werden diese vorab in die Vertexpositionen eingerechnet
        importFlags |= aiProcess_PreTransformVertices;
    }

    const aiScene* scene = aiImportFile(fullPath.c_str(), importFlags);

    if (scene == nullptr)
    {
        return false;
    }

    // Die Modell-Bounds werden direkt aus allen importierten Vertices bestimmt, damit Gameplay-Collider dieselben Abmessungen wie das verwendete Asset erhalten
    Vector minimum(1e20f, 1e20f, 1e20f);

    Vector maximum(-1e20f, -1e20f, -1e20f);

    bool hasVertices = false;

    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];

        if (mesh == nullptr || mesh->mVertices == nullptr)
        {
            continue;
        }

        for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
        {
            const aiVector3D& vertex = mesh->mVertices[vertexIndex];

            minimum.X = std::min(minimum.X, vertex.x);

            minimum.Y = std::min(minimum.Y, vertex.y);

            minimum.Z = std::min(minimum.Z, vertex.z);

            maximum.X = std::max(maximum.X, vertex.x);

            maximum.Y = std::max(maximum.Y, vertex.y);

            maximum.Z = std::max(maximum.Z, vertex.z);

            hasVertices = true;
        }
    }

    aiReleaseImport(scene);

    if (!hasVertices)
    {
        return false;
    }

    bounds = AABB(minimum, maximum);

    return true;
}

void AssetLoader::setAssetDirectory(const std::string& assetDirectory)
{
    m_assetDirectory = assetDirectory;

    if (m_assetDirectory.empty())
    {
        m_assetDirectory = defaultAssetDirectory();
    }

    char lastCharacter = m_assetDirectory.back();

    if (lastCharacter != '/' && lastCharacter != '\\')
    {
        m_assetDirectory += '/';
    }
}
