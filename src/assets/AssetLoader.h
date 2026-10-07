#pragma once

#include <string>
#include <vector>

class AABB;
class Model;

// Kapselt Asset-Pfade sowie das Laden von Modellen und deren Bounds
class AssetLoader
{
public:
    AssetLoader();

    explicit AssetLoader(const std::string& assetDirectory);

    std::string assetPath(const std::string& relativePath) const;

    std::string preferredAssetPath(const std::string& localRelativePath,
                                   const std::string& fallbackRelativePath) const;

    Model* loadModel(const std::string& relativePath, bool fitSize = true) const;

    bool loadModelBounds(const std::string& relativePath, AABB& bounds) const;

private:
    void setAssetDirectory(const std::string& assetDirectory);

    std::string m_assetDirectory;
};
