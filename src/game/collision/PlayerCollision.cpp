
#include "PlayerCollision.h"

#include <algorithm>

bool PlayerCollision::intersectsAny(const Vector& playerPosition,
                                    float playerRadius,
                                    const std::list<AABB>& colliders) const
{
    for (const AABB& collider : colliders)
    {
        if (intersects(playerPosition, playerRadius, collider))
        {
            return true;
        }
    }

    return false;
}

Vector PlayerCollision::move(const Vector& playerPosition,
                             const Vector& movement,
                             float playerRadius,
                             const std::list<AABB>& colliders) const
{
    Vector resultPosition = playerPosition;

    // X und Z werden nacheinander geprüft. Dadurch kann der Spieler an einer Wand entlanggleiten statt komplett stehen zu bleiben
    Vector newPositionX = playerPosition;

    newPositionX.X += movement.X;

    if (!intersectsAny(newPositionX, playerRadius, colliders))
    {
        resultPosition.X = newPositionX.X;
    }

    Vector newPositionZ = resultPosition;

    newPositionZ.Z += movement.Z;

    if (!intersectsAny(newPositionZ, playerRadius, colliders))
    {
        resultPosition.Z = newPositionZ.Z;
    }

    return resultPosition;
}

bool PlayerCollision::intersects(const Vector& playerPosition,
                                 float playerRadius,
                                 const AABB& collider) const
{
    // Der Spieler wird im Grundriss als Kreis behandelt; geprüft wird die Distanz zum nächsten Punkt der AABB
    float closestPointX = std::max(collider.Min.X, std::min(playerPosition.X, collider.Max.X));

    float closestPointZ = std::max(collider.Min.Z, std::min(playerPosition.Z, collider.Max.Z));

    float distanceX = playerPosition.X - closestPointX;

    float distanceZ = playerPosition.Z - closestPointZ;

    float distanceSquared = distanceX * distanceX + distanceZ * distanceZ;

    return distanceSquared <= playerRadius * playerRadius;
}
