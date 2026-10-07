#pragma once

#include "../../engine/Aabb.h"
#include "../../engine/Matrix.h"
#include "../../engine/vector.h"

#include "DeliveryBox.h"
#include "DeliveryZone.h"

#include <array>
#include <list>
#include <memory>
#include <vector>

class BaseModel;
class DeliveryBoxInteraction;
class IInteractable;
class Model;
class Product;
class AssetLoader;

// Bündelt Modelle, Animation, Collider und Interaktion des Lieferkartons in der Szene
class DeliveryBoxScene
{
public:
    explicit DeliveryBoxScene(AssetLoader& assetLoader);
    ~DeliveryBoxScene();

    void create(std::list<BaseModel*>& models,
                std::list<IInteractable*>& interactables,
                std::list<AABB>& interactionColliders);

    void update(float deltaTime, const std::vector<Product*>& products);

    DeliveryBox& deliveryBox();
    const DeliveryBox& deliveryBox() const;

    const DeliveryZone& deliveryZone() const;

    const Matrix& backLidPhysicsTransform() const;
    const Matrix& frontLidPhysicsTransform() const;
    const Vector& lidPhysicsSize() const;

    const std::vector<AABB>& carryColliders() const;

    const std::array<Matrix, 5>& bodyPhysicsTransforms() const;

    const std::array<Vector, 5>& bodyPhysicsSizes() const;

    bool consumeBlockedEvent();
    bool consumeSealedEvent();

    bool seal(int orderId, const std::vector<Product*>& products);

    std::vector<Product*> sealedProducts(const std::vector<Product*>& products) const;

    bool isInDeliveryZone() const;

    bool isPlayerNearDeliveryZone(const Vector& playerPosition) const;

    void reset();

    IInteractable* interaction() const;

    std::vector<BaseModel*> highlightModels() const;

private:
    void updateLidAnimation(float deltaTime, const std::vector<Product*>& products);

    void updateTransforms();

    void updateBodyPhysicsTransforms();

    void updateLidColliderVertices();

    bool isLidBlocked(const std::vector<Product*>& products) const;

    Matrix createLidPhysicsTransform(const Matrix& lidModelTransform) const;

    AssetLoader& m_assetLoader;

    DeliveryBox m_deliveryBox;
    DeliveryZone m_deliveryZone;

    Vector m_resetPosition;

    std::unique_ptr<DeliveryBoxInteraction> m_interaction;

    Model* m_bodyModel;
    Model* m_backLidModel;
    Model* m_frontLidModel;

    float m_openLidAngle;
    float m_lidAngle;
    float m_targetLidAngle;
    float m_lidSpeed;

    Matrix m_backLidTransform;
    Matrix m_frontLidTransform;

    std::vector<Vector> m_backLidWorldVertices;

    std::vector<Vector> m_frontLidWorldVertices;

    Matrix m_backLidPhysicsTransform;
    Matrix m_frontLidPhysicsTransform;

    Vector m_lidPhysicsSize;

    std::vector<AABB> m_carryColliders;

    std::array<Matrix, 5> m_bodyPhysicsTransforms;

    std::array<Vector, 5> m_bodyPhysicsSizes;

    bool m_blockedEvent;
    bool m_sealedEvent;
};
