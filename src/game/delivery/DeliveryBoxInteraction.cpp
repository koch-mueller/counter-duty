#include "DeliveryBoxInteraction.h"

DeliveryBoxInteraction::DeliveryBoxInteraction(DeliveryBox* deliveryBox)
    : m_deliveryBox(deliveryBox),
      m_hasLidColliders(false)
{
    updateCombinedCollider();
}

bool DeliveryBoxInteraction::canInteract() const
{
    if (m_deliveryBox == nullptr)
    {
        return false;
    }

    DeliveryBoxState state = m_deliveryBox->state();

    return state == DeliveryBoxState::Open
           || (state == DeliveryBoxState::Sealed && !m_deliveryBox->isBeingCarried());
}

std::string DeliveryBoxInteraction::getPromptText() const
{
    if (m_deliveryBox == nullptr)
    {
        return "";
    }

    if (m_deliveryBox->state() == DeliveryBoxState::Sealed)
    {
        return "[E] Karton aufnehmen";
    }

    return "[F] Karton schliessen";
}

void DeliveryBoxInteraction::interact()
{
    // Das Aufnehmen wird zentral über ICarryable und das CarrySystem verarbeitet
}

bool DeliveryBoxInteraction::secondaryInteract()
{
    if (m_deliveryBox == nullptr)
    {
        return false;
    }

    return m_deliveryBox->toggleLid();
}

const AABB& DeliveryBoxInteraction::getCollider() const
{
    return m_combinedCollider;
}

std::vector<AABB> DeliveryBoxInteraction::getInteractionColliders() const
{
    std::vector<AABB> colliders;

    if (m_deliveryBox == nullptr)
    {
        return colliders;
    }

    // Für den Raycast werden Boden/Wände und die animierten Deckel einzeln angeboten; so bleibt die Interaktion trotz offener Deckel zuverlässig
    const std::array<DeliveryBox::DeliveryBoxPart, 5>& parts = m_deliveryBox->parts();

    colliders.reserve(parts.size() + (m_hasLidColliders ? 2 : 0));

    for (const DeliveryBox::DeliveryBoxPart& part : parts)
    {
        colliders.push_back(AABB::fromCenterAndSize(part.center, part.size));
    }

    if (m_hasLidColliders)
    {
        colliders.push_back(m_backLidCollider);

        colliders.push_back(m_frontLidCollider);
    }

    return colliders;
}

int DeliveryBoxInteraction::interactionPriority() const
{
    return -1;
}

void DeliveryBoxInteraction::updateLidColliders(const std::vector<Vector>& backLidWorldVertices,
                                                const std::vector<Vector>& frontLidWorldVertices)
{
    if (backLidWorldVertices.empty() || frontLidWorldVertices.empty())
    {
        m_hasLidColliders = false;
        updateCombinedCollider();
        return;
    }

    m_backLidCollider.fromPoints(backLidWorldVertices.data(),
                                 static_cast<unsigned int>(backLidWorldVertices.size()));

    m_frontLidCollider.fromPoints(frontLidWorldVertices.data(),
                                  static_cast<unsigned int>(frontLidWorldVertices.size()));

    m_hasLidColliders = true;

    updateCombinedCollider();
}

bool DeliveryBoxInteraction::hasLidColliders() const
{
    return m_hasLidColliders;
}

const AABB& DeliveryBoxInteraction::backLidCollider() const
{
    return m_backLidCollider;
}

const AABB& DeliveryBoxInteraction::frontLidCollider() const
{
    return m_frontLidCollider;
}

bool DeliveryBoxInteraction::canBeCarried() const
{
    // Transport ist erst nach dem Versiegeln erlaubt, der offene Karton bleibt Bestandteil der Station
    return m_deliveryBox != nullptr && m_deliveryBox->state() == DeliveryBoxState::Sealed
           && !m_deliveryBox->isBeingCarried();
}

Matrix DeliveryBoxInteraction::getWorldTransform() const
{
    if (m_deliveryBox == nullptr)
    {
        Matrix transform;

        transform.identity();

        return transform;
    }

    return m_deliveryBox->worldTransform();
}

void DeliveryBoxInteraction::setWorldTransform(const Matrix& transform)
{
    if (m_deliveryBox != nullptr)
    {
        m_deliveryBox->setWorldTransform(transform);
    }
}

AABB DeliveryBoxInteraction::getLocalBounds() const
{
    if (m_deliveryBox == nullptr)
    {
        return AABB();
    }

    const AABB& outerBounds = m_deliveryBox->geometry().outerBounds;

    float worldScale = m_deliveryBox->worldScale();

    return AABB(outerBounds.Min * worldScale, outerBounds.Max * worldScale);
}

void DeliveryBoxInteraction::onCarryStarted()
{
    if (m_deliveryBox != nullptr)
    {
        m_deliveryBox->setBeingCarried(true);
    }
}

void DeliveryBoxInteraction::onCarryEnded()
{
    if (m_deliveryBox != nullptr)
    {
        m_deliveryBox->setBeingCarried(false);
    }
}

float DeliveryBoxInteraction::carryDistance() const
{
    return 1.7f;
}

float DeliveryBoxInteraction::minimumCarryDistance() const
{
    return 0.75f;
}

float DeliveryBoxInteraction::carryVerticalOffset() const
{
    return -0.45f;
}

bool DeliveryBoxInteraction::allowsCarryRotation() const
{
    return false;
}

bool DeliveryBoxInteraction::usesDynamicBlockingColliders() const
{
    return false;
}

bool DeliveryBoxInteraction::usesCameraRotation() const
{
    return false;
}

bool DeliveryBoxInteraction::requiresPlayerClearanceOnDrop() const
{
    return true;
}

void DeliveryBoxInteraction::updateCombinedCollider()
{
    std::vector<AABB> colliders = getInteractionColliders();

    if (colliders.empty())
    {
        m_combinedCollider = AABB();
        return;
    }

    m_combinedCollider = colliders.front();

    // Der kombinierte Collider dient als grobe Bounds; die präzise Auswahl nutzt weiterhin die Einzelcollider
    for (std::size_t index = 1; index < colliders.size(); ++index)
    {
        m_combinedCollider.merge(colliders[index]);
    }
}
