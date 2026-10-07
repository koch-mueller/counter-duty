#include "Product.h"

namespace
{
    constexpr float kDefaultMinimumCarryDistance = 0.1f;

    constexpr float kCameraMargin = 0.08f;
}

unsigned int Product::s_nextId = 1;

Product::Product(const ProductDefinition& definition, const Vector& position)
    : m_id(s_nextId++),
      m_definition(definition),
      m_state(ProductState::OnShelf),
      m_position(position),
      m_interactable(false),
      m_collider(AABB::fromCenterAndSize(position, definition.boundsSize())),
      m_isScanned(false),
      m_scannedOrderId(-1),
      m_minimumCarryDistance(kDefaultMinimumCarryDistance)
{
    m_worldTransform.translation(m_position);

    createLocalColliderVertices();
}

unsigned int Product::id() const
{
    return m_id;
}

const ProductDefinition& Product::definition() const
{
    return m_definition;
}

ProductState Product::state() const
{
    return m_state;
}

void Product::setState(ProductState state)
{
    m_state = state;
}

void Product::setPhysicsState(bool sleeping)
{
    if (m_state == ProductState::Held || m_state == ProductState::InSealedBox
        || m_state == ProductState::Delivered)
    {
        return;
    }

    // Ruhende und fallende Produkte teilen denselben Physikkörper; nur der Gameplay-Zustand spiegelt den Sleeping-Status wider
    if (sleeping)
    {
        m_state = ProductState::Sleeping;
    }
    else
    {
        m_state = ProductState::Dynamic;
    }
}

const Vector& Product::position() const
{
    return m_position;
}

void Product::setPosition(const Vector& position)
{
    m_position = position;

    m_worldTransform.translation(m_position);

    updateCollider();
}

bool Product::canInteract() const
{
    return m_interactable;
}

void Product::setInteractable(bool interactable)
{
    m_interactable = interactable;
}

std::string Product::getPromptText() const
{
    return "[E] " + m_definition.name() + " aufnehmen";
}

void Product::interact()
{
    // Das eigentliche Aufnehmen wird zentral durch InteractionController und CarrySystem ausgeführt
}

const AABB& Product::getCollider() const
{
    return m_collider;
}

bool Product::canBeCarried() const
{
    return m_state == ProductState::OnShelf || m_state == ProductState::Dynamic
           || m_state == ProductState::Sleeping;
}

Matrix Product::getWorldTransform() const
{
    return m_worldTransform;
}

void Product::setWorldTransform(const Matrix& transform)
{
    m_worldTransform = transform;

    m_position = transform.translation();

    updateCollider();
}

AABB Product::getLocalBounds() const
{
    Vector halfSize = m_definition.boundsSize() * 0.5f;

    return AABB(-halfSize, halfSize);
}

float Product::minimumCarryDistance() const
{
    return m_minimumCarryDistance;
}

void Product::onCarryStarted()
{
    m_state = ProductState::Held;

    m_interactable = false;
}

void Product::onCarryEnded()
{
    m_state = ProductState::Dynamic;

    m_interactable = true;
}

bool Product::isScanned() const
{
    return m_isScanned;
}

int Product::scannedOrderId() const
{
    return m_scannedOrderId;
}

void Product::markScanned(int orderId)
{
    m_scannedOrderId = orderId;

    m_isScanned = true;
}

Vector Product::barcodeWorldPosition() const
{
    return m_worldTransform * m_definition.barcodePosition();
}

Vector Product::barcodeWorldNormal() const
{
    // Für eine Richtung wird nur der 3x3-Rotationsteil transformiert, Translation darf die Normale nicht beeinflussen
    Vector worldNormal = m_worldTransform.transformVec3x3(m_definition.barcodeNormal());

    if (worldNormal.lengthSquared() > 0.000001f)
    {
        worldNormal.normalize();
    }

    return worldNormal;
}

const std::vector<Vector>& Product::localColliderVertices() const
{
    return m_localColliderVertices;
}

void Product::resetIdCounter()
{
    s_nextId = 1;
}

void Product::updateCollider()
{
    m_collider = getLocalBounds().transform(m_worldTransform);
}

void Product::createLocalColliderVertices()
{
    AABB localBounds = getLocalBounds();

    m_localColliderVertices.clear();

    m_localColliderVertices.reserve(8);

    float largestCornerDistance = 0.0f;

    for (int xIndex = 0; xIndex < 2; ++xIndex)
    {
        for (int yIndex = 0; yIndex < 2; ++yIndex)
        {
            for (int zIndex = 0; zIndex < 2; ++zIndex)
            {
                Vector vertex(xIndex == 0 ? localBounds.Min.X : localBounds.Max.X,
                              yIndex == 0 ? localBounds.Min.Y : localBounds.Max.Y,
                              zIndex == 0 ? localBounds.Min.Z : localBounds.Max.Z);

                m_localColliderVertices.push_back(vertex);

                float cornerDistance = vertex.length();

                if (cornerDistance > largestCornerDistance)
                {
                    largestCornerDistance = cornerDistance;
                }
            }
        }
    }

    // Die größte Ecke berücksichtigt auch ein beliebig gedrehtes Produkt vor der Kamera
    m_minimumCarryDistance = largestCornerDistance + kCameraMargin;
}
