#include "DeliveryZone.h"

DeliveryZone::DeliveryZone(const AABB& bounds, float interactionDistance)
    : m_bounds(bounds),
      m_interactionDistance(interactionDistance)
{
}

const AABB& DeliveryZone::bounds() const
{
    return m_bounds;
}

bool DeliveryZone::containsFully(const DeliveryBox& deliveryBox) const
{
    Vector localCorners[8];

    deliveryBox.geometry().outerBounds.corners(localCorners);

    Matrix scaleTransform;

    scaleTransform.scale(deliveryBox.worldScale());

    Matrix worldTransform = deliveryBox.worldTransform() * scaleTransform;

    // Der Karton gilt nur dann als abgegeben, wenn alle acht Ecken vollständig innerhalb der Zone liegen
    for (const Vector& localCorner : localCorners)
    {
        Vector worldCorner = worldTransform * localCorner;

        if (!m_bounds.contains(worldCorner))
        {
            return false;
        }
    }

    return true;
}

bool DeliveryZone::isPlayerNearby(const Vector& playerPosition) const
{
    float distanceX = 0.0f;

    if (playerPosition.X < m_bounds.Min.X)
    {
        distanceX = m_bounds.Min.X - playerPosition.X;
    }
    else if (playerPosition.X > m_bounds.Max.X)
    {
        distanceX = playerPosition.X - m_bounds.Max.X;
    }

    float distanceZ = 0.0f;

    if (playerPosition.Z < m_bounds.Min.Z)
    {
        distanceZ = m_bounds.Min.Z - playerPosition.Z;
    }
    else if (playerPosition.Z > m_bounds.Max.Z)
    {
        distanceZ = playerPosition.Z - m_bounds.Max.Z;
    }

    float squaredDistance = distanceX * distanceX + distanceZ * distanceZ;

    float squaredInteractionDistance = m_interactionDistance * m_interactionDistance;

    // Für die Nähe zur Abgabezone ist nur die Position auf dem Boden relevant, nicht die Höhe des Spielers
    return squaredDistance <= squaredInteractionDistance;
}
