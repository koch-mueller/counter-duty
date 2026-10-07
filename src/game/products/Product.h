#pragma once

#include "../carry/ICarryable.h"
#include "../interaction/IInteractable.h"

#include "ProductDefinition.h"
#include "ProductState.h"

#include <vector>

// Ein interaktives und tragbares Produkt mit Barcode, Zustand und Transform
class Product : public IInteractable, public ICarryable
{
public:
    Product(const ProductDefinition& definition, const Vector& position);

    unsigned int id() const;

    const ProductDefinition& definition() const;

    ProductState state() const;

    void setState(ProductState state);

    void setPhysicsState(bool sleeping);

    const Vector& position() const;

    void setPosition(const Vector& position);

    bool canInteract() const override;

    void setInteractable(bool interactable);

    std::string getPromptText() const override;

    void interact() override;

    const AABB& getCollider() const override;

    bool canBeCarried() const override;

    Matrix getWorldTransform() const override;

    void setWorldTransform(const Matrix& transform) override;

    AABB getLocalBounds() const override;

    float minimumCarryDistance() const override;

    void onCarryStarted() override;
    void onCarryEnded() override;

    bool isScanned() const;

    int scannedOrderId() const;

    void markScanned(int orderId);

    Vector barcodeWorldPosition() const;

    Vector barcodeWorldNormal() const;

    const std::vector<Vector>& localColliderVertices() const;

    static void resetIdCounter();

private:
    void updateCollider();

    void createLocalColliderVertices();

    static unsigned int s_nextId;

    unsigned int m_id;

    const ProductDefinition& m_definition;

    ProductState m_state;

    Vector m_position;

    bool m_interactable;

    AABB m_collider;

    Matrix m_worldTransform;

    bool m_isScanned;
    int m_scannedOrderId;

    std::vector<Vector> m_localColliderVertices;

    float m_minimumCarryDistance;
};
