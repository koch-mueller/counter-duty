#include "DeliveryBox.h"
#include "../collision/ConvexBoxCollision.h"

#include <utility>

DeliveryBox::DeliveryBox()
    : m_worldScale(1.0f),
      m_enterTolerance(0.015f),
      m_containedSideTolerance(0.015f),
      m_bottomTolerance(0.02f),
      m_topTolerance(0.015f),
      m_exitDelay(0.35f),
      m_state(DeliveryBoxState::Open),
      m_sealedOrderId(-1),
      m_isBeingCarried(false)
{
    m_worldTransform.identity();
}

void DeliveryBox::setGeometry(const DeliveryBoxGeometry& geometry,
                              const Vector& worldPosition,
                              float worldScale)
{
    m_geometry = geometry;
    m_worldPosition = worldPosition;
    m_worldScale = worldScale;
    m_state = DeliveryBoxState::Open;

    m_worldTransform.translation(worldPosition);

    updateWorldGeometry();

    m_containedProductIds.clear();
    m_outsideDurations.clear();
    m_sealedProductLocalTransforms.clear();

    m_sealedOrderId = -1;
    m_isBeingCarried = false;
}

const std::array<DeliveryBox::DeliveryBoxPart, 5>& DeliveryBox::parts() const
{
    return m_parts;
}

DeliveryBoxState DeliveryBox::state() const
{
    return m_state;
}

bool DeliveryBox::toggleLid()
{
    if (m_state != DeliveryBoxState::Open)
    {
        return false;
    }

    return startClosing();
}

bool DeliveryBox::startClosing()
{
    if (m_state != DeliveryBoxState::Open)
    {
        return false;
    }

    m_state = DeliveryBoxState::Closing;

    return true;
}

bool DeliveryBox::startOpening()
{
    if (m_state != DeliveryBoxState::Blocked)
    {
        return false;
    }

    m_state = DeliveryBoxState::Opening;

    return true;
}

void DeliveryBox::blockClosing()
{
    if (m_state == DeliveryBoxState::Closing)
    {
        m_state = DeliveryBoxState::Blocked;
    }
}

void DeliveryBox::finishClosing()
{
    if (m_state == DeliveryBoxState::Closing)
    {
        m_state = DeliveryBoxState::Sealed;
    }
}

void DeliveryBox::finishOpening()
{
    if (m_state == DeliveryBoxState::Opening || m_state == DeliveryBoxState::Blocked)
    {
        m_state = DeliveryBoxState::Open;
    }
}

const AABB& DeliveryBox::innerArea() const
{
    return m_innerArea;
}

const AABB& DeliveryBox::lidClearanceArea() const
{
    return m_lidClearanceArea;
}

const DeliveryBoxGeometry& DeliveryBox::geometry() const
{
    return m_geometry;
}

const Vector& DeliveryBox::worldPosition() const
{
    return m_worldPosition;
}

float DeliveryBox::worldScale() const
{
    return m_worldScale;
}

bool DeliveryBox::contains(const std::vector<Vector>& localColliderVertices,
                           const Matrix& worldTransform,
                           float sideTolerance,
                           float bottomTolerance,
                           float topTolerance) const
{
    if (localColliderVertices.empty())
    {
        return false;
    }

    // Ein Produkt zählt erst als vollständig enthalten, wenn jeder Collider-Eckpunkt innerhalb der tolerierten Innenbox liegt
    for (const Vector& localPoint : localColliderVertices)
    {
        Vector worldPoint = worldTransform * localPoint;

        if (!containsWorldPoint(worldPoint, sideTolerance, bottomTolerance, topTolerance))
        {
            return false;
        }
    }

    return true;
}

bool DeliveryBox::overlapsInnerArea(const std::vector<Vector>& localColliderVertices,
                                    const Matrix& worldTransform) const
{
    if (localColliderVertices.empty())
    {
        return false;
    }

    std::vector<Vector> productWorldVertices =
        ConvexBoxCollision::transformVertices(localColliderVertices, worldTransform);

    Vector localInnerVertices[8];

    m_geometry.innerBounds.corners(localInnerVertices);

    Matrix scaleTransform;

    scaleTransform.scale(m_worldScale);

    Matrix innerWorldTransform = m_worldTransform * scaleTransform;

    std::vector<Vector> innerWorldVertices;

    innerWorldVertices.reserve(8);

    for (const Vector& localInnerVertex : localInnerVertices)
    {
        innerWorldVertices.push_back(innerWorldTransform * localInnerVertex);
    }

    // Eine reine Berührung am Kartonrand zählt noch nicht als teilweise enthalten
    return ConvexBoxCollision::overlaps(productWorldVertices, innerWorldVertices, -0.001f);
}

bool DeliveryBox::overlapsLidClearance(const std::vector<Vector>& localVertices,
                                       const Matrix& worldTransform) const
{
    if (localVertices.empty())
    {
        return false;
    }

    std::vector<Vector> worldVertices;

    worldVertices.reserve(localVertices.size());

    for (const Vector& localVertex : localVertices)
    {
        worldVertices.push_back(worldTransform * localVertex);
    }

    AABB worldBounds;

    worldBounds.fromPoints(worldVertices.data(), static_cast<unsigned int>(worldVertices.size()));

    return overlapsLidClearance(worldBounds);
}

bool DeliveryBox::overlapsLidClearance(const AABB& bounds) const
{
    return bounds.overlaps(m_lidClearanceArea);
}

void DeliveryBox::updateContents(const std::vector<Product*>& products, float deltaTime)
{
    if (m_state == DeliveryBoxState::Sealed)
    {
        updateSealedContents(products);
        return;
    }

    std::unordered_set<int> updatedProductIds;

    for (const Product* product : products)
    {
        if (product == nullptr)
        {
            continue;
        }

        int productId = static_cast<int>(product->id());

        bool wasContained = containsProductId(productId);

        if (product->state() == ProductState::Held)
        {
            m_outsideDurations.erase(productId);
            continue;
        }

        // Beim Eintritt gilt eine strengere Toleranz als bei bereits enthaltenen Produkten. Das erzeugt Hysterese gegen Physikzittern
        float sideTolerance = wasContained ? m_containedSideTolerance : m_enterTolerance;

        bool isContained = contains(product->localColliderVertices(),
                                    product->getWorldTransform(),
                                    sideTolerance,
                                    m_bottomTolerance,
                                    m_topTolerance);

        if (isContained)
        {
            updatedProductIds.insert(productId);
            m_outsideDurations.erase(productId);
            continue;
        }

        if (!wasContained)
        {
            m_outsideDurations.erase(productId);
            continue;
        }

        float& outsideDuration = m_outsideDurations[productId];

        outsideDuration += deltaTime;

        // Eine kurze Verzögerung verhindert, dass kleine Physikbewegungen den Kartoninhalt ständig zwischen innen und außen wechseln lassen
        if (outsideDuration < m_exitDelay)
        {
            updatedProductIds.insert(productId);
        }
        else
        {
            m_outsideDurations.erase(productId);
        }
    }

    m_containedProductIds = std::move(updatedProductIds);
}

bool DeliveryBox::containsProductId(int productId) const
{
    return m_containedProductIds.find(productId) != m_containedProductIds.end();
}

const std::unordered_set<int>& DeliveryBox::containedProductIds() const
{
    return m_containedProductIds;
}

const Matrix& DeliveryBox::worldTransform() const
{
    return m_worldTransform;
}

void DeliveryBox::setWorldTransform(const Matrix& transform)
{
    m_worldTransform = transform;
    m_worldPosition = transform.translation();

    updateWorldGeometry();
}

bool DeliveryBox::sealContents(int orderId, const std::vector<Product*>& products)
{
    if (m_state != DeliveryBoxState::Sealed || orderId < 0)
    {
        return false;
    }

    // Beim Versiegeln werden die Produkttransforms einmal relativ zum Karton gespeichert
    Matrix inverseBoxTransform = m_worldTransform;

    inverseBoxTransform.invert();

    m_sealedProductLocalTransforms.clear();

    for (Product* product : products)
    {
        if (product == nullptr || !containsProductId(static_cast<int>(product->id())))
        {
            continue;
        }

        Matrix localTransform = inverseBoxTransform * product->getWorldTransform();

        m_sealedProductLocalTransforms[product->id()] = localTransform;

        product->setState(ProductState::InSealedBox);

        product->setInteractable(false);
    }

    if (m_sealedProductLocalTransforms.empty())
    {
        return false;
    }

    m_sealedOrderId = orderId;

    return true;
}

void DeliveryBox::updateSealedContents(const std::vector<Product*>& products) const
{
    if (m_state != DeliveryBoxState::Sealed)
    {
        return;
    }

    for (Product* product : products)
    {
        if (product == nullptr)
        {
            continue;
        }

        auto transformEntry = m_sealedProductLocalTransforms.find(product->id());

        if (transformEntry == m_sealedProductLocalTransforms.end())
        {
            continue;
        }

        // Die Produkte bleiben relativ zum Karton gespeichert und folgen dadurch seiner Bewegung, ohne eigene Physik zu benötigen
        Matrix productWorldTransform = m_worldTransform * transformEntry->second;

        product->setWorldTransform(productWorldTransform);

        product->setState(ProductState::InSealedBox);

        product->setInteractable(false);
    }
}

std::vector<Product*> DeliveryBox::sealedProducts(const std::vector<Product*>& products) const
{
    std::vector<Product*> result;

    result.reserve(m_sealedProductLocalTransforms.size());

    for (Product* product : products)
    {
        if (product == nullptr)
        {
            continue;
        }

        if (m_sealedProductLocalTransforms.find(product->id())
            != m_sealedProductLocalTransforms.end())
        {
            result.push_back(product);
        }
    }

    return result;
}

int DeliveryBox::sealedOrderId() const
{
    return m_sealedOrderId;
}

bool DeliveryBox::isBeingCarried() const
{
    return m_isBeingCarried;
}

void DeliveryBox::setBeingCarried(bool beingCarried)
{
    m_isBeingCarried = beingCarried;
}

void DeliveryBox::resetForNextOrder(const Vector& worldPosition)
{
    m_state = DeliveryBoxState::Open;
    m_sealedOrderId = -1;
    m_isBeingCarried = false;

    m_containedProductIds.clear();
    m_outsideDurations.clear();
    m_sealedProductLocalTransforms.clear();

    Matrix worldTransform;

    worldTransform.translation(worldPosition);

    setWorldTransform(worldTransform);
}

bool DeliveryBox::containsWorldPoint(const Vector& worldPoint,
                                     float sideTolerance,
                                     float bottomTolerance,
                                     float topTolerance) const
{
    if (m_worldScale <= 0.0f)
    {
        return false;
    }

    // Die Punktprüfung erfolgt in lokaler Kartongeometrie, damit sie auch bei gedrehtem/transportiertem Karton korrekt bleibt
    Matrix inverseWorldTransform = m_worldTransform;

    inverseWorldTransform.invert();

    float inverseScale = 1.0f / m_worldScale;

    Vector localPoint = inverseWorldTransform * worldPoint;

    localPoint = localPoint * inverseScale;

    float localSideTolerance = sideTolerance * inverseScale;

    float localBottomTolerance = bottomTolerance * inverseScale;

    float localTopTolerance = topTolerance * inverseScale;

    AABB toleratedInnerBounds(
        m_geometry.innerBounds.Min
            - Vector(localSideTolerance, localBottomTolerance, localSideTolerance),
        m_geometry.innerBounds.Max
            + Vector(localSideTolerance, localTopTolerance, localSideTolerance));

    return toleratedInnerBounds.contains(localPoint);
}

void DeliveryBox::updateWorldGeometry()
{
    Matrix scaleTransform;

    scaleTransform.scale(m_worldScale);

    Matrix assetWorldTransform = m_worldTransform * scaleTransform;

    m_innerArea = m_geometry.innerBounds.transform(assetWorldTransform);

    m_lidClearanceArea = m_geometry.lidClearanceBounds.transform(assetWorldTransform);

    for (std::size_t index = 0; index < m_parts.size(); ++index)
    {
        const DeliveryBoxGeometry::Part& geometryPart = m_geometry.parts[index];

        AABB localBounds = AABB::fromCenterAndSize(geometryPart.center, geometryPart.size);

        AABB worldBounds = localBounds.transform(assetWorldTransform);

        m_parts[index].center = worldBounds.center();

        m_parts[index].size = worldBounds.size();
    }

    // Die Toleranz für bereits enthaltene Produkte orientiert sich an der realen Wandstärke des skalierten Kartons
    float wallThickness = m_geometry.parts[1].size.X * m_worldScale;

    m_containedSideTolerance = wallThickness + 0.015f;
}
