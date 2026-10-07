#pragma once

#include "../../assets/PhongMaterial.h"
#include "../../engine/Aabb.h"

#include <list>

class AssetLoader;
class BaseModel;
class Texture;
class Model;

// Erzeugt die statische Ladenumgebung samt Modellen, Materialien, Licht und Grund-Collidern
class StoreEnvironmentBuilder
{
public:
    explicit StoreEnvironmentBuilder(const AssetLoader& assetLoader);

    void create(std::list<BaseModel*>& models,
                std::list<AABB>& playerColliders,
                std::list<AABB>& interactionColliders,
                std::list<AABB>& physicsColliders);

    const AABB& terminalBounds() const;

    Model* createCleaningToolVisual(Matrix& visualOffset, Vector& toolSize) const;

    BaseModel* createSpillVisual() const;

    BaseModel* terminalVisual() const;

private:
    bool initializeTerminalAsset();

    void createFloor(std::list<BaseModel*>& models, std::list<AABB>& physicsColliders) const;

    void createRoof(std::list<BaseModel*>& models) const;

    void createWalls(std::list<BaseModel*>& models,
                     std::list<AABB>& playerColliders,
                     std::list<AABB>& interactionColliders,
                     std::list<AABB>& physicsColliders) const;

    void createDeliveryArea(std::list<BaseModel*>& models) const;

    void createTerminal(std::list<BaseModel*>& models,
                        std::list<AABB>& playerColliders,
                        std::list<AABB>& physicsColliders);

    void createLights() const;

    void createMopStation(std::list<BaseModel*>& models,
                          std::list<AABB>& playerColliders,
                          std::list<AABB>& physicsColliders) const;

    void createCeilingLights(std::list<BaseModel*>& models) const;

    const AssetLoader& m_assetLoader;

    BaseModel* m_terminalVisual;

    AABB m_terminalModelBounds;

    Vector m_terminalModelScale;

    AABB m_terminalBounds;

    bool m_terminalAssetReady;

    PhongMaterial m_floorMaterial;
    PhongMaterial m_wallMaterial;
    PhongMaterial m_ceilingMaterial;
    PhongMaterial m_deliveryZoneMaterial;
};
