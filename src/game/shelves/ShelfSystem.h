#pragma once

#include "../../engine/Aabb.h"

#include "../products/ProductCatalog.h"
#include "../products/ProductCollection.h"
#include "../rendering/ProductVisualSystem.h"

#include "Shelf.h"

#include <array>
#include <cstddef>
#include <list>
#include <vector>

class AssetLoader;
class BaseModel;

// Erzeugt Regale und Produktreihen und koordiniert Entnahme, Nachfüllen und Vorschieben
class ShelfSystem
{
public:
    ShelfSystem(const AssetLoader& assetLoader,
                const ProductCatalog& productCatalog,
                ProductCollection& products,
                ProductVisualSystem& productVisualSystem);

    void create(std::list<BaseModel*>& models,
                std::list<AABB>& playerColliders,
                std::list<AABB>& interactionColliders,
                std::list<AABB>& physicsColliders);

    void update(float deltaTime,
                std::list<BaseModel*>& models,
                std::list<IInteractable*>& interactables,
                std::vector<IInteractable*>& pendingInteractables);
    
    bool consumeRefillCompletedEvent();

private:
    bool initializeShelfAsset();

    Vector shelfModelCenter() const;

    float centeredScaledY(float localY) const;

    float centeredScaledZ(float localZ) const;

    float shelfSurfaceLocalY(std::size_t surfaceIndex, float localZ) const;

    float shelfUsableWidth() const;

    float shelfUsableDepth() const;

    float shelfBackWallDepth() const;

    BaseModel* createShelfVisual(const Vector& center, float frontDirectionZ) const;

    void createOpenShelf(const Vector& center,
                         float frontDirectionZ,
                         std::list<BaseModel*>& models,
                         std::list<AABB>& playerColliders,
                         std::list<AABB>& interactionColliders,
                         std::list<AABB>& physicsColliders);

    std::vector<Vector> createSlotPositions(const Vector& shelfCenter,
                                            float frontDirectionZ,
                                            std::size_t surfaceIndex,
                                            const Vector& productSize) const;

    void createProductRows(const Vector& shelfCenter,
                           float frontDirectionZ,
                           const std::array<ProductType, 4>& productTypes,
                           std::list<BaseModel*>& models);

    Vector createSpawnPosition(const std::vector<Vector>& slotPositions) const;

    void createInitialProducts(ShelfProductRow& row, std::list<BaseModel*>& models);

    void refillRows(std::list<BaseModel*>& models,
                    std::list<IInteractable*>& interactables,
                    std::vector<IInteractable*>& pendingInteractables);

    const AssetLoader& m_assetLoader;

    const ProductCatalog& m_productCatalog;

    ProductCollection& m_products;

    ProductVisualSystem& m_productVisualSystem;

    AABB m_shelfModelBounds;

    Vector m_shelfModelScale;

    bool m_shelfAssetReady;

    std::vector<Shelf> m_shelves;
    
    bool m_refillCompletedEvent;
};
