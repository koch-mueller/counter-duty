#pragma once

#include "../../engine/Aabb.h"

#include "../delivery/DeliveryBoxScene.h"
#include "../interaction/IInteractable.h"
#include "../products/ProductCatalog.h"
#include "../products/ProductCollection.h"
#include "../rendering/ProductVisualSystem.h"
#include "../scanner/ScannerStation.h"
#include "../shelves/ShelfSystem.h"
#include "StoreEnvironmentBuilder.h"
#include "../cleaning/CleaningTool.h"
#include "../cleaning/Spill.h"
#include "../cleaning/CleaningSystem.h"

#include <array>
#include <list>
#include <vector>

class AssetLoader;
class BaseModel;
class Model;
class HighlightRenderer;

// Fasst Ladenumgebung und Gameplay-Teilsysteme zusammen und synchronisiert deren gemeinsame Szene
class StoreScene
{
public:
    explicit StoreScene(AssetLoader& assetLoader);

    void create(std::list<BaseModel*>& models);

    void update(float deltaTime, std::list<BaseModel*>& models);

    const std::list<AABB>& staticColliders() const;

    const std::list<AABB>& interactionColliders() const;

    const std::list<AABB>& physicsColliders() const;

    const std::list<IInteractable*>& interactables() const;

    const std::vector<IInteractable*>& pendingInteractables() const;

    void clearPendingInteractables();

    const ProductCatalog& productCatalog() const;

    ScannerStation& scannerStation();

    const ScannerStation& scannerStation() const;

    DeliveryBox& deliveryBox();

    const DeliveryBox& deliveryBox() const;

    const DeliveryZone& deliveryZone() const;

    const std::vector<Product*>& products() const;

    const Matrix& deliveryBoxBackLidPhysicsTransform() const;

    const Matrix& deliveryBoxFrontLidPhysicsTransform() const;

    const Vector& deliveryBoxLidPhysicsSize() const;

    const std::vector<AABB>& deliveryBoxCarryColliders() const;

    const std::array<Matrix, 5>& deliveryBoxBodyPhysicsTransforms() const;

    const std::array<Vector, 5>& deliveryBoxBodyPhysicsSizes() const;

    bool consumeDeliveryBoxBlockedEvent();

    bool consumeDeliveryBoxSealedEvent();

    bool sealDeliveryBox(int orderId);

    std::vector<Product*> sealedDeliveryBoxProducts() const;

    bool isDeliveryBoxInDeliveryZone() const;

    bool isPlayerNearDeliveryZone(const Vector& playerPosition) const;

    void removeDeliveredProducts(const std::vector<Product*>& deliveredProducts,
                                 std::list<BaseModel*>& models);

    void resetDeliveryBox();

    const AABB& terminalBounds() const;

    void registerHighlightModels(HighlightRenderer& highlighter) const;

    BaseModel* terminalVisual() const;
    
    bool canCleanSpill() const;

    bool cleanSpill(float deltaTime);
    
    bool isSpillActive() const;
    
    bool consumeShelfRefillCompletedEvent();

    bool consumeSpillStartedEvent();

    const std::vector<AABB>& playerDynamicColliders() const;

    bool hasCleaningTool() const;

    Matrix cleaningToolTransform() const;

    Vector cleaningToolPhysicsSize() const;

    const std::vector<AABB>& carryBlockingColliders() const;

private:
    void registerAllProducts();

    void updateSpillVisual();

    void updatePlayerDynamicColliders();

    void updateCarryBlockingColliders();

    std::vector<AABB> m_carryBlockingColliders;

    std::list<AABB> m_staticColliders;
    std::list<AABB> m_interactionColliders;
    std::list<AABB> m_physicsColliders;

    std::list<IInteractable*> m_interactables;

    std::vector<IInteractable*> m_pendingInteractables;

    ProductCatalog m_productCatalog;
    ProductCollection m_products;

    ProductVisualSystem m_productVisualSystem;

    StoreEnvironmentBuilder m_environmentBuilder;

    ScannerStation m_scannerStation;

    ShelfSystem m_shelfSystem;

    DeliveryBoxScene m_deliveryBoxScene;

    CleaningTool m_cleaningTool;

    Model* m_cleaningToolModel;

    Matrix m_cleaningToolVisualOffset;

    // Nur fuer die sichtbare Wischbewegung, der echte Tool-Transform bleibt davon getrennt
    float m_cleaningAnimationTime;
    bool m_cleaningAnimationActive;

    Spill m_spill;
    
    CleaningSystem m_cleaningSystem;

    BaseModel* m_spillModel;

    std::vector<AABB> m_playerDynamicColliders;
};
