#pragma once

#include "../../engine/Aabb.h"
#include "../../engine/Matrix.h"
#include "../../engine/vector.h"
#include "../products/Product.h"
#include "DeliveryBoxGeometry.h"

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Zustandsautomat des Lieferkartons während Öffnen, Schließen und Versiegeln
enum class DeliveryBoxState
{
    Open,
    Closing,
    Blocked,
    Opening,
    Sealed
};

// Enthält Zustand, Geometrie und Inhaltsprüfung des Lieferkartons
// Versiegelte Produkte werden relativ zum Karton gespeichert und mit ihm mitgeführt
class DeliveryBox
{
public:
    // Beschreibt einen einzelnen Modell-/Physikteil des Kartons mit lokalem Transform und Bounds
struct DeliveryBoxPart
    {
        Vector center;
        Vector size;
    };

    DeliveryBox();

    void setGeometry(const DeliveryBoxGeometry& geometry,
                     const Vector& worldPosition,
                     float worldScale);

    const std::array<DeliveryBoxPart, 5>& parts() const;

    DeliveryBoxState state() const;

    bool toggleLid();
    bool startClosing();
    bool startOpening();

    void blockClosing();
    void finishClosing();
    void finishOpening();

    const AABB& innerArea() const;
    const AABB& lidClearanceArea() const;

    const DeliveryBoxGeometry& geometry() const;
    const Vector& worldPosition() const;
    float worldScale() const;

    bool contains(const std::vector<Vector>& localColliderVertices,
                  const Matrix& worldTransform,
                  float sideTolerance,
                  float bottomTolerance,
                  float topTolerance) const;

    bool overlapsInnerArea(const std::vector<Vector>& localColliderVertices,
                           const Matrix& worldTransform) const;

    bool overlapsLidClearance(const std::vector<Vector>& localVertices,
                              const Matrix& worldTransform) const;

    bool overlapsLidClearance(const AABB& bounds) const;

    void updateContents(const std::vector<Product*>& products, float deltaTime);

    bool containsProductId(int productId) const;

    const std::unordered_set<int>& containedProductIds() const;

    const Matrix& worldTransform() const;
    void setWorldTransform(const Matrix& transform);

    bool sealContents(int orderId, const std::vector<Product*>& products);

    void updateSealedContents(const std::vector<Product*>& products) const;

    std::vector<Product*> sealedProducts(const std::vector<Product*>& products) const;

    int sealedOrderId() const;

    bool isBeingCarried() const;
    void setBeingCarried(bool beingCarried);

    void resetForNextOrder(const Vector& worldPosition);

private:
    bool containsWorldPoint(const Vector& worldPoint,
                            float sideTolerance,
                            float bottomTolerance,
                            float topTolerance) const;

    void updateWorldGeometry();

    DeliveryBoxGeometry m_geometry;
    Vector m_worldPosition;
    float m_worldScale;

    std::array<DeliveryBoxPart, 5> m_parts;

    AABB m_innerArea;
    AABB m_lidClearanceArea;

    float m_enterTolerance;
    float m_containedSideTolerance;
    float m_bottomTolerance;
    float m_topTolerance;
    float m_exitDelay;

    std::unordered_set<int> m_containedProductIds;
    std::unordered_map<int, float> m_outsideDurations;

    DeliveryBoxState m_state;
    Matrix m_worldTransform;

    int m_sealedOrderId;
    std::unordered_map<unsigned int, Matrix> m_sealedProductLocalTransforms;
    bool m_isBeingCarried;
};
