#pragma once

#include "../../engine/Aabb.h"

#include <list>

// Behandelt die horizontale Spielerkollision gegen AABB-Hindernisse
class PlayerCollision
{
public:
    bool intersectsAny(const Vector& playerPosition,
                       float playerRadius,
                       const std::list<AABB>& colliders) const;

    Vector move(const Vector& playerPosition,
                const Vector& movement,
                float playerRadius,
                const std::list<AABB>& colliders) const;

private:
    bool intersects(const Vector& playerPosition, float playerRadius, const AABB& collider) const;
};
