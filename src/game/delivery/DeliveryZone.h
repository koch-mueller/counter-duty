#pragma once

#include "../../engine/Aabb.h"
#include "DeliveryBox.h"

// Beschreibt den Bereich, in dem ein versiegelter Karton abgegeben werden kann
class DeliveryZone
{
public:
    DeliveryZone(const AABB& bounds, float interactionDistance);

    const AABB& bounds() const;

    bool containsFully(const DeliveryBox& deliveryBox) const;

    bool isPlayerNearby(const Vector& playerPosition) const;

private:
    AABB m_bounds;
    float m_interactionDistance;
};
